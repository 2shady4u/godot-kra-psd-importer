#include "gdpsd.h"

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

#include "Samples/PsdTgaExporter.h"

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

void PSD::_register_methods()
{

    register_method("export_psd", &PSD::export_psd);

    register_property<PSD, String>("psd_file_path", &PSD::psd_file_path, "res://addons/godot-psd-importer/examples/Sample.psd");
	register_property<PSD, String>("target_folder_path", &PSD::target_folder_path, "res://graphics");

	register_property<PSD, bool>("verbose_mode", &PSD::verbose_mode, false);
}

PSD::PSD()
{
}

PSD::~PSD()
{
}

void PSD::_init()
{
    Godot::print("- initializing C++ -");
	verbose_mode = false;
}

int PSD::export_psd()
{
    Godot::print("- Exporting PSD to TGA -");

    /* Find the real path */
    psd_file_path = ProjectSettings::get_singleton()->globalize_path(psd_file_path.strip_edges());
	/* Convert to the necessary std::wstring */
	const std::wstring srcPath = psd_file_path.unicode_str();

    target_folder_path = ProjectSettings::get_singleton()->globalize_path(target_folder_path.strip_edges());
	const std::wstring targetFolder = target_folder_path.unicode_str();

	MallocAllocator allocator;
	NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenRead(srcPath.c_str()))
	{
		OutputDebugStringA("Cannot open file.\n");
		return 1;
	}

    // create a new document that can be used for extracting different sections from the PSD.
	// additionally, the document stores information like width, height, bits per pixel, etc.
	Document* document = CreateDocument(&file, &allocator);
	if (!document)
	{
		OutputDebugStringA("Cannot create document.\n");
		file.Close();
		return 1;
	}

	// the sample only supports RGB colormode
	if (document->colorMode != colorMode::RGB)
	{
		OutputDebugStringA("Document is not in RGB color mode.\n");
		DestroyDocument(document, &allocator);
		file.Close();
		return 1;
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

			// note that channel data is only as big as the layer it belongs to, e.g. it can be smaller or bigger than the canvas,
			// depending on where it is positioned. therefore, we use the provided utility functions to expand/shrink the channel data
			// to the canvas size. of course, you can work with the channel data directly if you need to.
			void* canvasData[4] = {};
			unsigned int channelCount = 0u;
			if ((indexR != CHANNEL_NOT_FOUND) && (indexG != CHANNEL_NOT_FOUND) && (indexB != CHANNEL_NOT_FOUND))
			{
				// RGB channels were found.
				canvasData[0] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexR]);
				canvasData[1] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexG]);
				canvasData[2] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexB]);
				channelCount = 3u;

				if (indexA != CHANNEL_NOT_FOUND)
				{
					// A channel was also found.
					canvasData[3] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexA]);
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
					image8 = CreateInterleavedImage<uint8_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], document->width, document->height);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], document->width, document->height);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], document->width, document->height);
				}
			}
			else if (channelCount == 4u)
			{
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], document->width, document->height);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], document->width, document->height);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], document->width, document->height);
				}
			}

			allocator.Free(canvasData[0]);
			allocator.Free(canvasData[1]);
			allocator.Free(canvasData[2]);
			allocator.Free(canvasData[3]);

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
					filename << L"layer";
					filename << layerName.str();
					filename << L".tga";
					tgaExporter::SaveRGB(filename.str().c_str(), document->width, document->height, image8);
				}
			}
			else if (channelCount == 4u)
			{
				if (document->bitsPerChannel == 8u)
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << L"layer";
					filename << layerName.str();
					filename << L".tga";
					tgaExporter::SaveRGBA(filename.str().c_str(), document->width, document->height, image8);
				}
			}

			allocator.Free(image8);
			allocator.Free(image16);
			allocator.Free(image32);

			// in addition to the layer data, we also want to extract the user and/or vector mask.
			// luckily, this has been handled already by the ExtractLayer() function. we just need to check whether a mask exists.
			if (layer->layerMask)
			{
				// a layer mask exists, and data is available. work out the mask's dimensions.
				const unsigned int width = static_cast<unsigned int>(layer->layerMask->right - layer->layerMask->left);
				const unsigned int height = static_cast<unsigned int>(layer->layerMask->bottom - layer->layerMask->top);

				// similar to layer data, the mask data can be smaller or bigger than the canvas.
				// the mask data is always single-channel (monochrome), and has a width and height as calculated above.
				void* maskData = layer->layerMask->data;
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << L"layer";
					filename << layerName.str();
					filename << L"_usermask.tga";
					tgaExporter::SaveMonochrome(filename.str().c_str(), width, height, static_cast<const uint8_t*>(maskData));
				}

				// use ExpandMaskToCanvas create an image that is the same size as the canvas.
				void* maskCanvasData = ExpandMaskToCanvas(document, &allocator, layer->layerMask);
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << L"canvas";
					filename << layerName.str();
					filename << L"_usermask.tga";
					tgaExporter::SaveMonochrome(filename.str().c_str(), document->width, document->height, static_cast<const uint8_t*>(maskCanvasData));
				}

				allocator.Free(maskCanvasData);
			}

			if (layer->vectorMask)
			{
				// accessing the vector mask works exactly like accessing the layer mask.
				const unsigned int width = static_cast<unsigned int>(layer->vectorMask->right - layer->vectorMask->left);
				const unsigned int height = static_cast<unsigned int>(layer->vectorMask->bottom - layer->vectorMask->top);

				void* maskData = layer->vectorMask->data;
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << L"layer";
					filename << layerName.str();
					filename << L"_vectormask.tga";
					tgaExporter::SaveMonochrome(filename.str().c_str(), width, height, static_cast<const uint8_t*>(maskData));
				}

				void* maskCanvasData = ExpandMaskToCanvas(document, &allocator, layer->vectorMask);
				{
					std::wstringstream filename;
					filename << targetFolder;
					filename << L"canvas";
					filename << layerName.str();
					filename << L"_vectormask.tga";
					tgaExporter::SaveMonochrome(filename.str().c_str(), document->width, document->height, static_cast<const uint8_t*>(maskCanvasData));
				}

				allocator.Free(maskCanvasData);
			}
		}

		DestroyLayerMaskSection(layerMaskSection, &allocator);
	}

	// extract the image data section, if available. the image data section stores the final, merged image, as well as additional
	// alpha channels. this is only available when saving the document with "Maximize Compatibility" turned on.
	if (document->imageDataSection.length != 0)
	{
		ImageDataSection* imageData = ParseImageDataSection(document, &file, &allocator);
		if (imageData)
		{
			// interleave the planar image data into one RGB or RGBA image.
			// store the rest of the (alpha) channels and the transparency mask separately.
			const unsigned int imageCount = imageData->imageCount;

			// note that an image can have more than 3 channels, but still no transparency mask in case all extra channels
			// are actual alpha channels.
			bool isRgb = false;
			if (imageCount == 3)
			{
				// imageData->images[0], imageData->images[1] and imageData->images[2] contain the R, G, and B channels of the merged image.
				// they are always the size of the canvas/document, so we can interleave them using imageUtil::InterleaveRGB directly.
				isRgb = true;
			}
			else if (imageCount >= 4)
			{
				// check if we really have a transparency mask that belongs to the "main" merged image.
				if (hasTransparencyMask)
				{
					// we have 4 or more images/channels, and a transparency mask.
					// this means that images 0-3 are RGBA, respectively.
					isRgb = false;
				}
				else
				{
					// we have 4 or more images stored in the document, but none of them is the transparency mask.
					// this means we are dealing with RGB (!) data, and several additional alpha channels.
					isRgb = true;
				}
			}

			uint8_t* image8 = nullptr;
			uint16_t* image16 = nullptr;
			float32_t* image32 = nullptr;
			if (isRgb)
			{
				// RGB
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, document->width, document->height);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, document->width, document->height);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, document->width, document->height);
				}
			}
			else
			{
				// RGBA
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, imageData->images[3].data, document->width, document->height);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, imageData->images[3].data, document->width, document->height);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, imageData->images[0].data, imageData->images[1].data, imageData->images[2].data, imageData->images[3].data, document->width, document->height);
				}
			}

			if (document->bitsPerChannel == 8)
			{
				std::wstringstream filename;
				filename << targetFolder;
				filename << L"merged.tga";
				if (isRgb)
				{
					tgaExporter::SaveRGB(filename.str().c_str(), document->width, document->height, image8);
				}
				else
				{
					tgaExporter::SaveRGBA(filename.str().c_str(), document->width, document->height, image8);
				}
			}

			allocator.Free(image8);
			allocator.Free(image16);
			allocator.Free(image32);

			// extract image resources in order to acquire the alpha channel names.
			ImageResourcesSection* imageResources = ParseImageResourcesSection(document, &file, &allocator);
			if (imageResources)
			{
				// store all the extra alpha channels. in case we have a transparency mask, it will always be the first of the
				// extra channels.
				// alpha channel names can be accessed using imageResources->alphaChannels[index].
				// loop through all alpha channels, and skip all channels that were already merged (either RGB or RGBA).
				const unsigned int skipImageCount = isRgb ? 3u : 4u;
				for (unsigned int i = 0u; i < imageCount - skipImageCount; ++i)
				{
					AlphaChannel* channel = imageResources->alphaChannels + i;

					if (document->bitsPerChannel == 8)
					{
						std::wstringstream filename;
						filename << targetFolder;
						filename << L"extra_channel_";
						filename << channel->asciiName.c_str();
						filename << L".tga";

						tgaExporter::SaveMonochrome(filename.str().c_str(), document->width, document->height, static_cast<const uint8_t*>(imageData->images[i + skipImageCount].data));
					}
				}

				DestroyImageResourcesSection(imageResources, &allocator);
			}

			DestroyImageDataSection(imageData, &allocator);
		}
	}

	// don't forget to destroy the document, and close the file.
	DestroyDocument(document, &allocator);
	file.Close();

	return 0;
}