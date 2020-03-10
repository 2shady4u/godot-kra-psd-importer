#include "gdpsdimporter.h"

// the main include that always needs to be included in every translation unit that uses the PSD library
#include "Psd/Psd.h"

// for convenience reasons, we directly include the platform header from the PSD library.
// we could have just included <Windows.h> as well, but that is unnecessarily big, and triggers lots of warnings.
#include "Psd/PsdPlatform.h"

// in the sample, we use the provided malloc allocator for all memory allocations. likewise, we also use the provided
// native file interface.
// in your code, feel free to use whatever allocator you have lying around.
#include "Psd/PsdMallocAllocator.h"
#include "Psd/PsdNativeFile.h"

#include "Psd/PsdDocument.h"
#include "Psd/PsdColorMode.h"
#include "Psd/PsdLayer.h"
#include "Psd/PsdChannel.h"
#include "Psd/PsdChannelType.h"
#include "Psd/PsdLayerMask.h"
#include "Psd/PsdVectorMask.h"
#include "Psd/PsdLayerMaskSection.h"
#include "Psd/PsdImageDataSection.h"
#include "Psd/PsdImageResourcesSection.h"
#include "Psd/PsdParseDocument.h"
#include "Psd/PsdParseLayerMaskSection.h"
#include "Psd/PsdParseImageDataSection.h"
#include "Psd/PsdParseImageResourcesSection.h"
#include "Psd/PsdLayerCanvasCopy.h"
#include "Psd/PsdInterleave.h"
#include "Psd/PsdPlanarImage.h"
#include "Psd/PsdExport.h"
#include "Psd/PsdExportDocument.h"

#include "exporters/PsdTgaExporter.h"
#include "exporters/PsdPngExporter.h"

PSD_PUSH_WARNING_LEVEL(0)
	// disable annoying warning caused by xlocale(337): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
	#pragma warning(disable:4530)
	#include <string>
	#include <sstream>
PSD_POP_WARNING_LEVEL

PSD_USING_NAMESPACE;


// helpers for reading PSDs
namespace
{
	static const unsigned int CHANNEL_NOT_FOUND = UINT_MAX;


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T, typename DataHolder>
	static void* ExpandChannelToCanvas(Allocator* allocator, const DataHolder* layer, const void* data, unsigned int canvasWidth, unsigned int canvasHeight)
	{
		T* canvasData = static_cast<T*>(allocator->Allocate(sizeof(T)*canvasWidth*canvasHeight, 16u));
		memset(canvasData, 0u, sizeof(T)*canvasWidth*canvasHeight);

		imageUtil::CopyLayerData(static_cast<const T*>(data), canvasData, layer->left, layer->top, layer->right, layer->bottom, canvasWidth, canvasHeight);

		return canvasData;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	static void* ExpandChannelToCanvas(const Document* document, Allocator* allocator, Layer* layer, Channel* channel)
	{
		if (document->bitsPerChannel == 8)
			return ExpandChannelToCanvas<uint8_t>(allocator, layer, channel->data, document->width, document->height);
		else if (document->bitsPerChannel == 16)
			return ExpandChannelToCanvas<uint16_t>(allocator, layer, channel->data, document->width, document->height);
		else if (document->bitsPerChannel == 32)
			return ExpandChannelToCanvas<float32_t>(allocator, layer, channel->data, document->width, document->height);

		return nullptr;
	}

//canvasData[0] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexR]);


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	static void* ExpandMaskToCanvas(const Document* document, Allocator* allocator, T* mask)
	{
		if (document->bitsPerChannel == 8)
			return ExpandChannelToCanvas<uint8_t>(allocator, mask, mask->data, document->width, document->height);
		else if (document->bitsPerChannel == 16)
			return ExpandChannelToCanvas<uint16_t>(allocator, mask, mask->data, document->width, document->height);
		else if (document->bitsPerChannel == 32)
			return ExpandChannelToCanvas<float32_t>(allocator, mask, mask->data, document->width, document->height);

		return nullptr;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	unsigned int FindChannel(Layer* layer, int16_t channelType)
	{
		for (unsigned int i = 0; i < layer->channelCount; ++i)
		{
			Channel* channel = &layer->channels[i];
			if (channel->data && channel->type == channelType)
				return i;
		}

		return CHANNEL_NOT_FOUND;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	T* CreateInterleavedImage(Allocator* allocator, const void* srcR, const void* srcG, const void* srcB, unsigned int width, unsigned int height)
	{
		T* image = static_cast<T*>(allocator->Allocate(width*height * 4u*sizeof(T), 16u));

		const T* r = static_cast<const T*>(srcR);
		const T* g = static_cast<const T*>(srcG);
		const T* b = static_cast<const T*>(srcB);
		imageUtil::InterleaveRGB(r, g, b, T(0), image, width, height);

		return image;
	}


	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	template <typename T>
	T* CreateInterleavedImage(Allocator* allocator, const void* srcR, const void* srcG, const void* srcB, const void* srcA, unsigned int width, unsigned int height)
	{
		T* image = static_cast<T*>(allocator->Allocate(width*height * 4u*sizeof(T), 16u));

		const T* r = static_cast<const T*>(srcR);
		const T* g = static_cast<const T*>(srcG);
		const T* b = static_cast<const T*>(srcB);
		const T* a = static_cast<const T*>(srcA);
		imageUtil::InterleaveRGBA(r, g, b, a, image, width, height);

		return image;
	}
}

using namespace godot;

void PSDImporter::_register_methods()
{

    register_method("export_all_layers", &PSDImporter::export_all_layers);
	register_method("test", &PSDImporter::test);

    register_property<PSDImporter, String>("psd_file_path", &PSDImporter::psd_file_path, "res://addons/godot-psd-importer/examples/Sample.psd");
	register_property<PSDImporter, String>("target_folder_path", &PSDImporter::target_folder_path, "res://");
	register_property<PSDImporter, String>("error_message", &PSDImporter::error_message, "");

	register_property<PSDImporter, bool>("verbose_mode", &PSDImporter::verbose_mode, false);
	register_property<PSDImporter, bool>("crop_to_canvas", &PSDImporter::crop_to_canvas, true);

	register_property<PSDImporter, int>("export_type", &PSDImporter::export_type, ExportType::PNG);

	register_signal<PSDImporter>("texture_created", "texture_path", GODOT_VARIANT_TYPE_STRING, "position", GODOT_VARIANT_TYPE_VECTOR2);
}

PSDImporter::PSDImporter()
{
}

PSDImporter::~PSDImporter()
{
}

void PSDImporter::_init()
{
	verbose_mode = false;

	// Verify if the environment variable 'MAGICK_CODER_MODULE_PATH' is set.
	String var = "MAGICK_CODER_MODULE_PATH";
	char *value = getenv(var.alloc_c_string());
	if(value)
	{
		Godot::print("Environment variable " + var + " points to path '" + value + "'");
	}
	else
	{
		Godot::print("GDPSDImporter warning: Environment variable " + var + " is not set in the system registery! (Ignore warning if ImageMagick is installed)");
	}

}

int PSDImporter::test()
{

	unsigned char pix[]={200,200,200, 100,100,100, 0,0,0, 255,0,0, 0,255,0, 0,0,255};

	// Initialise ImageMagick library
	Magick::InitializeMagick(NULL);

	Godot::print("after init");

   	// Create Image object and read in from pixel data above
	Magick::Image image;
	image.read(2, 3, "RGB", MagickCore::CharPixel, pix);

	// Write the image to a file - change extension if you want a GIF or JPEG
	image.magick("png"); 
	image.write("result.png");

	return 0;
}

bool PSDImporter::export_all_layers()
{
	if (verbose_mode) 
	{
		Godot::print("Exporting all layers..");
	}

    /* Find the real path */
    psd_file_path = ProjectSettings::get_singleton()->globalize_path(psd_file_path.strip_edges());
	/* Convert to the necessary std::wstring */
	const std::wstring srcPath = psd_file_path.unicode_str();

	/* Find the real path */
    target_folder_path = ProjectSettings::get_singleton()->globalize_path(target_folder_path.strip_edges());
	/* Check if this folder exists */
	Directory *dir = Directory::_new();
	if (!dir->dir_exists(target_folder_path)) {
		error_message = "Target directory does not exist!";
		Godot::print("GDPSDImporter Error: " + error_message);
		return false;
	}
	/* Convert to the necessary std::wstring */
	const std::wstring targetFolder = target_folder_path.unicode_str();

	MallocAllocator allocator;
	NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenRead(srcPath.c_str()))
	{
		error_message = "PSD file cannot be opened!";
		Godot::print("GDPSDImporter Error: " + error_message);
		return false;
	}

    // create a new document that can be used for extracting different sections from the PSD.
	// additionally, the document stores information like width, height, bits per pixel, etc.
	Document* document = CreateDocument(&file, &allocator);
	if (!document)
	{
		error_message = "PSD document cannot be created!";
		Godot::print("GDPSDImporter Error: " + error_message);
		file.Close();
		return false;
	}

	// the sample only supports RGB colormode
	if (document->colorMode != colorMode::RGB)
	{
		error_message = "PSD document is not in RGB color mode!";
		Godot::print("GDPSDImporter Error: " + error_message);
		DestroyDocument(document, &allocator);
		file.Close();
		return false;
	}

	// extract image resources section.
	// this gives access to the ICC profile, EXIF data and XMP metadata.
	{
		ImageResourcesSection* imageResourcesSection = ParseImageResourcesSection(document, &file, &allocator);
		OutputDebugStringA("XMP metadata:\n");
		OutputDebugStringA(imageResourcesSection->xmpMetadata);
		OutputDebugStringA("\n");
		DestroyImageResourcesSection(imageResourcesSection, &allocator);
	}

	// extract all layers and masks.
	bool hasTransparencyMask = false;
	LayerMaskSection* layerMaskSection = ParseLayerMaskSection(document, &file, &allocator);
	if (layerMaskSection)
	{
		hasTransparencyMask = layerMaskSection->hasTransparencyMask;

		// extract all layers one by one. this should be done in parallel for maximum efficiency.
		for (unsigned int i = 0; i < layerMaskSection->layerCount; ++i)
		{
			Layer* layer = &layerMaskSection->layers[i];
			ExtractLayer(document, &file, &allocator, layer);

			// check availability of R, G, B, and A channels.
			// we need to determine the indices of channels individually, because there is no guarantee that R is the first channel,
			// G is the second, B is the third, and so on.
			const unsigned int indexR = FindChannel(layer, channelType::R);
			const unsigned int indexG = FindChannel(layer, channelType::G);
			const unsigned int indexB = FindChannel(layer, channelType::B);
			const unsigned int indexA = FindChannel(layer, channelType::TRANSPARENCY_MASK);

			unsigned int layer_width = document->width;
			unsigned int layer_height = document->height;
			if (crop_to_canvas == false)
			{
				layer_width = (unsigned int)(layer->bottom - layer->top);
				layer_height = (unsigned int)(layer->right - layer->left);
			}

			// note that channel data is only as big as the layer it belongs to, e.g. it can be smaller or bigger than the canvas,
			// depending on where it is positioned. therefore, we use the provided utility functions to expand/shrink the channel data
			// to the canvas size. of course, you can work with the channel data directly if you need to.
			void* canvasData[4] = {};
			unsigned int channelCount = 0u;
			if ((indexR != CHANNEL_NOT_FOUND) && (indexG != CHANNEL_NOT_FOUND) && (indexB != CHANNEL_NOT_FOUND))
			{
				// RGB channels were found.
				if (crop_to_canvas == true)
				{
					canvasData[0] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexR]);
					canvasData[1] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexG]);
					canvasData[2] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexB]);
				}
				else
				{
					canvasData[0] = layer->channels[indexR].data;
					canvasData[1] = layer->channels[indexG].data;
					canvasData[2] = layer->channels[indexB].data;
				}
				channelCount = 3u;

				if (indexA != CHANNEL_NOT_FOUND)
				{
					// A channel was also found.
					if (crop_to_canvas == true)
					{
						canvasData[3] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexA]);
					}
					else
					{
						canvasData[3] = layer->channels[indexA].data;
					}
					channelCount = 4u;
				}
			}

			// interleave the different pieces of planar canvas data into one RGB or RGBA image, depending on what channels
			// we found, and what color mode the document is stored in.
			uint8_t* image8 = nullptr;
			uint16_t* image16 = nullptr;
			float32_t* image32 = nullptr;
			if (channelCount == 3u)
			{
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], layer_width, layer_height);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], layer_width, layer_height);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], layer_width, layer_height);
				}
			}
			else if (channelCount == 4u)
			{
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], layer_width, layer_height);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], layer_width, layer_height);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], layer_width, layer_height);
				}
			}

			// ONLY free canvasData if the channel was actually copied! Otherwise the channel data is already deleted here!
			if (crop_to_canvas == true)
			{
				allocator.Free(canvasData[0]);
				allocator.Free(canvasData[1]);
				allocator.Free(canvasData[2]);
				allocator.Free(canvasData[3]);
			}

			// get the layer name.
			// Unicode data is preferred because it is not truncated by Photoshop, but unfortunately it is optional.
			// fall back to the ASCII name in case no Unicode name was found.
			std::wstringstream layerName;
			if (layer->utf16Name)
			{
				layerName << reinterpret_cast<wchar_t*>(layer->utf16Name);
			}
			else
			{
				layerName << layer->name.c_str();
			}

			// at this point, image8, image16 or image32 store either a 8-bit, 16-bit, or 32-bit image, respectively.
			// the image data is stored in interleaved RGB or RGBA, and has the size "document->width*document->height".
			// it is up to you to do whatever you want with the image data. in the sample, we simply write the image to a .TGA file.
			if (channelCount == 3u)
			{
				if (document->bitsPerChannel == 8u)
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << layerName.str();
					switch (export_type)
					{
					case PNG:
						filename << L".png";
						Godot::print(">> Exporting RGB layer '" + String(layerName.str().c_str()) + "' to PNG ('" + String(filename.str().c_str()) + "')");
						try
						{
							pngExporter::SaveRGB(filename.str().c_str(), layer_width, layer_height, image8);
						}
						catch(const std::exception& e)
						{
							// Verify the environment variable 'MAGICK_CODER_MODULE_PATH'.
							String var = "MAGICK_CODER_MODULE_PATH";
							char *value = getenv(var.alloc_c_string());
							if(!value)
							{
								error_message = var + " is missing!";
							}
							else
							{
								error_message = "Unknown error (check console)";
							}
							Godot::print("GDPSDImporter Error (ImageMagick):" + String(e.what()));
							return false;
						}
						break;

					case TGA:
						filename << L".tga";
						Godot::print(">> Exporting RGB layer '" + String(layerName.str().c_str()) + "' to TGA ('" + String(filename.str().c_str()) + "')");
						tgaExporter::SaveRGB(filename.str().c_str(), layer_width, layer_height, image8);
						break;

					default:
						break;
					}
					std::wstring dummy = filename.str();
					emit_signal("texture_created", String(dummy.c_str()), Vector2(layer->left ,layer->top));
				}
			}
			else if (channelCount == 4u)
			{
				if (document->bitsPerChannel == 8u)
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << layerName.str();
					switch (export_type)
					{
					case PNG:
						filename << L".png";
						Godot::print(">> Exporting RGBA layer '" + String(layerName.str().c_str()) + "' to PNG ('" + String(filename.str().c_str()) + "')");
						try
						{
							pngExporter::SaveRGBA(filename.str().c_str(), layer_width, layer_height, image8);
						}
						catch(const std::exception& e)
						{
							// Verify the environment variable 'MAGICK_CODER_MODULE_PATH'.
							String var = "MAGICK_CODER_MODULE_PATH";
							char *value = getenv(var.alloc_c_string());
							if(!value)
							{
								error_message = var + " is missing!";
							}
							else
							{
								error_message = "Unknown error (check console)";
							}
							Godot::print("GDPSDImporter Error (ImageMagick):" + String(e.what()));
							return false;
						}
						break;
					case TGA:
						filename << L".tga";
						Godot::print(">> Exporting RGBA layer '" + String(layerName.str().c_str()) + "' to PNG ('" + String(filename.str().c_str()) + "')");
						tgaExporter::SaveRGBA(filename.str().c_str(), layer_width, layer_height, image8);
						break;

					default:
						break;
					}
					std::wstring dummy = filename.str();
					emit_signal("texture_created", String(dummy.c_str()), Vector2(layer->left ,layer->top));
				}
			}

			allocator.Free(image8);
			allocator.Free(image16);
			allocator.Free(image32);
		}
		DestroyLayerMaskSection(layerMaskSection, &allocator);
	}

	// don't forget to destroy the document, and close the file.
	DestroyDocument(document, &allocator);
	file.Close();

	return true;
}