#include "gdkrapsdimporter.h"

KRA_USING_NAMESPACE;
#ifdef WIN32
PSD_USING_NAMESPACE;

// helpers for reading PSDs
namespace
{
static const unsigned int CHANNEL_NOT_FOUND = UINT_MAX;

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T, typename DataHolder>
static void *ExpandChannelToCanvas(Allocator *allocator, const DataHolder *layer, const void *data, unsigned int canvasWidth, unsigned int canvasHeight)
{
	T *canvasData = static_cast<T *>(allocator->Allocate(sizeof(T) * canvasWidth * canvasHeight, 16u));
	memset(canvasData, 0u, sizeof(T) * canvasWidth * canvasHeight);

	imageUtil::CopyLayerData(static_cast<const T *>(data), canvasData, layer->left, layer->top, layer->right, layer->bottom, canvasWidth, canvasHeight);

	return canvasData;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
static void *ExpandChannelToCanvas(const Document *document, Allocator *allocator, Layer *layer, Channel *channel)
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
static void *ExpandMaskToCanvas(const Document *document, Allocator *allocator, T *mask)
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
unsigned int FindChannel(Layer *layer, int16_t channelType)
{
	for (unsigned int i = 0; i < layer->channelCount; ++i)
	{
		Channel *channel = &layer->channels[i];
		if (channel->data && channel->type == channelType)
			return i;
	}

	return CHANNEL_NOT_FOUND;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
T *CreateInterleavedImage(Allocator *allocator, const void *srcR, const void *srcG, const void *srcB, unsigned int width, unsigned int height)
{
	T *image = static_cast<T *>(allocator->Allocate(width * height * 4u * sizeof(T), 16u));

	const T *r = static_cast<const T *>(srcR);
	const T *g = static_cast<const T *>(srcG);
	const T *b = static_cast<const T *>(srcB);
	imageUtil::InterleaveRGB(r, g, b, T(0), image, width, height);

	return image;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
template <typename T>
T *CreateInterleavedImage(Allocator *allocator, const void *srcR, const void *srcG, const void *srcB, const void *srcA, unsigned int width, unsigned int height)
{
	T *image = static_cast<T *>(allocator->Allocate(width * height * 4u * sizeof(T), 16u));

	const T *r = static_cast<const T *>(srcR);
	const T *g = static_cast<const T *>(srcG);
	const T *b = static_cast<const T *>(srcB);
	const T *a = static_cast<const T *>(srcA);
	imageUtil::InterleaveRGBA(r, g, b, a, image, width, height);

	return image;
}
} // namespace
#endif

using namespace godot;

void KRAPSDImporter::_register_methods()
{

	register_method("export_all_layers", &KRAPSDImporter::ExportAllLayers);

	register_property<KRAPSDImporter, String>("raw_file_path", &KRAPSDImporter::rawFilePath, "res://addons/godot-psd-importer/examples/PSDExample.psd");
	register_property<KRAPSDImporter, String>("target_folder_path", &KRAPSDImporter::targetFolderPath, "res://");
	register_property<KRAPSDImporter, String>("error_message", &KRAPSDImporter::errorMessage, "");

	register_property<KRAPSDImporter, bool>("verbose_mode", &KRAPSDImporter::verboseMode, false);
	register_property<KRAPSDImporter, bool>("crop_to_canvas", &KRAPSDImporter::cropToCanvas, true);
	register_property<KRAPSDImporter, bool>("mirror_universe", &KRAPSDImporter::mirrorUniverse, false);

	//register_property<KRAPSDImporter, int>("export_type", &KRAPSDImporter::exportType, EXPORT_TYPE::PNG);
	register_property<KRAPSDImporter, float>("resize_factor", &KRAPSDImporter::resizeFactor, 1);

	register_signal<KRAPSDImporter>("texture_created", "texture_properties", GODOT_VARIANT_TYPE_DICTIONARY);
}

KRAPSDImporter::KRAPSDImporter()
{
}

KRAPSDImporter::~KRAPSDImporter()
{
}

void KRAPSDImporter::_init()
{
	verboseMode = false;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool KRAPSDImporter::ExportAllLayers()
{
	if (verboseMode)
	{
		Godot::print("Exporting all layers...");
	}

	/* Find the real path */
	rawFilePath = ProjectSettings::get_singleton()->globalize_path(rawFilePath.strip_edges());

	/* Find the real path */
	targetFolderPath = ProjectSettings::get_singleton()->globalize_path(targetFolderPath.strip_edges());
	/* Check if the target folder exists */
	Ref<Directory> dir = Directory::_new();
	if (!dir->dir_exists(targetFolderPath))
	{
		errorMessage = "Target directory does not exist!";
		Godot::print("GDKRAPSDImporter Error: " + errorMessage);
		return false;
	}

	/* Find out if it is a KRA- or PSD-file that is being imported */
	String extension = rawFilePath.get_extension();

	if (extension == "kra")
	{
		if (verboseMode)
		{
			Godot::print("(GDKRAPSDImporter) Detected KRA-format... running KRA export!");
		}
		importType = IMPORT_TYPE::KRA;
		return ExportAllKRALayers();
	}
	else if (extension == "psd")
	{
		#ifdef WIN32
		if (verboseMode)
		{
			Godot::print("(GDKRAPSDImporter) Detected PSD-format... running PSD export!");
		}
		importType = IMPORT_TYPE::PSD;
		return ExportAllPSDLayers();
		#endif
		errorMessage = "This format is not supported on this platform!";
		Godot::print("GDKRAPSDImporter Error: Detected PSD-format... " + errorMessage);
		return false;
	}
	else
	{
		errorMessage = "File extension with name '" + extension + "' is invalid!";
		Godot::print("GDKRAPSDImporter Error: " + errorMessage);
		return false;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
bool KRAPSDImporter::ExportAllKRALayers()
{

	/* Convert to the necessary std::wstring */
	const std::wstring rawFile = rawFilePath.unicode_str();
	/* Convert to the necessary std::wstring */
	const std::wstring targetFolder = targetFolderPath.unicode_str();

	KraDocument* document = CreateKraDocument(rawFile);

	std::vector<KraExportedLayer*> exportedLayers = CreateKraExportLayers(document);

	for (auto const& layer : exportedLayers)
	{
		const wchar_t* layerName = layer->name;
		unsigned int layerHeight = (unsigned int)(layer->bottom - layer->top);
		unsigned int layerWidth = (unsigned int)(layer->right - layer->left);
		const uint8_t *data = layer->data;
		switch (layer->channelCount)
		{
		case 4u:
			channelType = COLOR_SPACE_NAME::RGBA;
			break;
		case 3u:
			channelType = COLOR_SPACE_NAME::RGB;
			break;
		
		default:
			break;
		}
		std::wstring filename = ExportLayer(layerName, layerWidth, layerHeight, data);
		/* TODO: we should check if the file actually exists! */
		if (mirrorUniverse)
		{
			/* Emit the texture properties to Godot if a mirror universe is requested */
			bool success = EmitKRATextureProperties(filename, layer);
		}

	}

	/* Clean up the memory allocation of our structs */
	DestroyKraExportLayers(exportedLayers);
	DestroyKraDocument(document);

	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
#ifdef WIN32
bool KRAPSDImporter::ExportAllPSDLayers()
{

	/* Convert to the necessary std::wstring */
	const std::wstring rawFile = rawFilePath.unicode_str();

	MallocAllocator allocator;
	NativeFile file(&allocator);

	// try opening the file. if it fails, bail out.
	if (!file.OpenRead(rawFile.c_str()))
	{
		errorMessage = "PSD file cannot be opened!";
		Godot::print("GDPSDImporter Error: " + errorMessage);
		return false;
	}

	// create a new document that can be used for extracting different sections from the PSD.
	// additionally, the document stores information like width, height, bits per pixel, etc.
	Document *document = CreateDocument(&file, &allocator);
	if (!document)
	{
		errorMessage = "PSD document cannot be created!";
		Godot::print("GDPSDImporter Error: " + errorMessage);
		file.Close();
		return false;
	}

	// the sample only supports RGB colormode
	if (document->colorMode != colorMode::RGB)
	{
		errorMessage = "PSD document is not in RGB color mode!";
		Godot::print("GDPSDImporter Error: " + errorMessage);
		DestroyDocument(document, &allocator);
		file.Close();
		return false;
	}

	// extract image resources section.
	// this gives access to the ICC profile, EXIF data and XMP metadata.
	{
		ImageResourcesSection *imageResourcesSection = ParseImageResourcesSection(document, &file, &allocator);
		OutputDebugStringA("XMP metadata:\n");
		OutputDebugStringA(imageResourcesSection->xmpMetadata);
		OutputDebugStringA("\n");
		DestroyImageResourcesSection(imageResourcesSection, &allocator);
	}

	// extract all layers and masks.
	bool hasTransparencyMask = false;
	LayerMaskSection *layerMaskSection = ParseLayerMaskSection(document, &file, &allocator);
	if (layerMaskSection)
	{
		hasTransparencyMask = layerMaskSection->hasTransparencyMask;

		// extract all layers one by one. this should be done in parallel for maximum efficiency.
		for (unsigned int i = 0; i < layerMaskSection->layerCount; ++i)
		{
			Layer *layer = &layerMaskSection->layers[i];
			if (layer->type != layerType::ANY)
			{
				Godot::print("(GDKRAPSDImporter) Ignoring non-exportable layer");
				continue;
			}
			ExtractLayer(document, &file, &allocator, layer);

			// check availability of R, G, B, and A channels.
			// we need to determine the indices of channels individually, because there is no guarantee that R is the first channel,
			// G is the second, B is the third, and so on.
			const unsigned int indexR = FindChannel(layer, channelType::R);
			const unsigned int indexG = FindChannel(layer, channelType::G);
			const unsigned int indexB = FindChannel(layer, channelType::B);
			const unsigned int indexA = FindChannel(layer, channelType::TRANSPARENCY_MASK);

			unsigned int layerWidth = document->width;
			unsigned int layerHeight = document->height;
			if (cropToCanvas == false)
			{
				layerHeight = (unsigned int)(layer->bottom - layer->top);
				layerWidth = (unsigned int)(layer->right - layer->left);
			}

			// note that channel data is only as big as the layer it belongs to, e.g. it can be smaller or bigger than the canvas,
			// depending on where it is positioned. therefore, we use the provided utility functions to expand/shrink the channel data
			// to the canvas size. of course, you can work with the channel data directly if you need to.
			void *canvasData[4] = {};
			unsigned int channelCount = 0u;
			if ((indexR != CHANNEL_NOT_FOUND) && (indexG != CHANNEL_NOT_FOUND) && (indexB != CHANNEL_NOT_FOUND))
			{
				// RGB channels were found.
				if (cropToCanvas == true)
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
				channelType = COLOR_SPACE_NAME::RGB;

				if (indexA != CHANNEL_NOT_FOUND)
				{
					// A channel was also found.
					if (cropToCanvas == true)
					{
						canvasData[3] = ExpandChannelToCanvas(document, &allocator, layer, &layer->channels[indexA]);
					}
					else
					{
						canvasData[3] = layer->channels[indexA].data;
					}
					channelCount = 4u;
					channelType = COLOR_SPACE_NAME::RGBA;
				}
			}

			// interleave the different pieces of planar canvas data into one RGB or RGBA image, depending on what channels
			// we found, and what color mode the document is stored in.
			uint8_t *image8 = nullptr;
			uint16_t *image16 = nullptr;
			float32_t *image32 = nullptr;
			if (channelCount == 3u)
			{
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], layerWidth, layerHeight);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], layerWidth, layerHeight);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], layerWidth, layerHeight);
				}
			}
			else if (channelCount == 4u)
			{
				if (document->bitsPerChannel == 8)
				{
					image8 = CreateInterleavedImage<uint8_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], layerWidth, layerHeight);
				}
				else if (document->bitsPerChannel == 16)
				{
					image16 = CreateInterleavedImage<uint16_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], layerWidth, layerHeight);
				}
				else if (document->bitsPerChannel == 32)
				{
					image32 = CreateInterleavedImage<float32_t>(&allocator, canvasData[0], canvasData[1], canvasData[2], canvasData[3], layerWidth, layerHeight);
				}
			}

			// ONLY free canvasData if the channel was actually copied! Otherwise the channel data is already deleted here!
			if (cropToCanvas == true)
			{
				allocator.Free(canvasData[0]);
				allocator.Free(canvasData[1]);
				allocator.Free(canvasData[2]);
				allocator.Free(canvasData[3]);
			}

			// get the layer name.
			// Unicode data is preferred because it is not truncated by Photoshop, but unfortunately it is optional.
			// fall back to the ASCII name in case no Unicode name was found.
			std::wstringstream ssLayerName;
			if (layer->utf16Name)
			{
				ssLayerName << reinterpret_cast<wchar_t *>(layer->utf16Name);
			}
			else
			{
				ssLayerName << layer->name.c_str();
			}
			/* Do this cast in two parts, otherwise weird undefined behaviour happens */
			std::wstring wslayerName = ssLayerName.str();
			const wchar_t* layerName = wslayerName.c_str();

			// at this point, image8, image16 or image32 store either a 8-bit, 16-bit, or 32-bit image, respectively.
			// the image data is stored in interleaved RGB or RGBA, and has the size "document->width*document->height".
			// it is up to you to do whatever you want with the image data. in the sample, we simply write the image to a .TGA file.
			if (document->bitsPerChannel == 8u)
			{
				std::wstring filename = ExportLayer(layerName, layerWidth, layerHeight, image8);
				if (mirrorUniverse)
				{
					bool success = EmitPSDTextureProperties(filename, layer);
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
#endif

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::wstring KRAPSDImporter::ExportLayer(const wchar_t* name, unsigned int width, unsigned int height, const uint8_t *data)
{
	/* Convert to the necessary std::wstring */
	const std::wstring targetFolder = targetFolderPath.unicode_str();
	std::wstringstream ssFilename;
	ssFilename << targetFolder;
	ssFilename << name;
	const char *channelChar = "Error";
	switch (channelType)
	{
	case MONOCHROME:
		channelChar = "Monochrome";
		break;
	case RGB:
		channelChar = "RGB";
		break;
	case RGBA:
		channelChar = "RGBA";
		break;
	default:
		break;
	}
	//switch (exportType)
	//{
	//case PNG:
		ssFilename << L".png";
		Godot::print("(GDKRAPSDImporter) Saving " + String(channelChar) + " layer '" + String(name) + "' to PNG ('" + String(ssFilename.str().c_str()) + "')");
		//break;

	//case TGA:
	//	ssFilename << L".tga";
	//	Godot::print("(GDKRAPSDImporter) Saving " + String(channelChar) + " layer '" + String(name) + "' to TGA ('" + String(ssFilename.str().c_str()) + "')");
	//	break;

	//default:
	//	break;
	//}

	std::wstring filename = ssFilename.str();

	if (!SaveTexture(ssFilename.str().c_str(), width, height, data))
	{

	}
	return filename;
}

// ---------------------------------------------------------------------------------------------------------------------
// Create a Dictionary that contains all of the texture properties as derived from the kra::KraExportedLayer
// ---------------------------------------------------------------------------------------------------------------------
bool KRAPSDImporter::EmitKRATextureProperties(std::wstring filename, KraExportedLayer* layer)
{
	Dictionary textureProperties = Dictionary();
	textureProperties["path"] = String(filename.c_str());
	/* Cropping to the canvas doesn't work yet for KRA-files! */
	/* This variable doesn't have any effect yet! */
	if (cropToCanvas == false || true)
	{
		/* Don't forget to include scaling here! */
		textureProperties["position"] = resizeFactor*Vector2((real_t)layer->left, (real_t)layer->top);
	}
	else
	{
		textureProperties["position"] = Vector2();
	}
	emit_signal("texture_created", textureProperties);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// Create a Dictionary that contains all of the texture properties as derived from the psd::Layer
// ---------------------------------------------------------------------------------------------------------------------
#ifdef WIN32
bool KRAPSDImporter::EmitPSDTextureProperties(std::wstring filename, Layer* layer)
{
	Dictionary textureProperties = Dictionary();
	textureProperties["path"] = String(filename.c_str());
	if (cropToCanvas == false)
	{
		/* Don't forget to include scaling here! */
		textureProperties["position"] = resizeFactor*Vector2((real_t)layer->left, (real_t)layer->top);
	}
	else
	{
		textureProperties["position"] = Vector2();
	}
	emit_signal("texture_created", textureProperties);
	return true;
}
#endif

// ---------------------------------------------------------------------------------------------------------------------
// Here the data gets exported and saved as a PNG using the libpng library.
// ---------------------------------------------------------------------------------------------------------------------
bool KRAPSDImporter::SaveTexture(const wchar_t *filename, unsigned int width, unsigned int height, const uint8_t *data)
{

	bool success = true;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	std::wstring ws(filename);
	std::string str(ws.begin(), ws.end());
	const char * cFilename = str.c_str();

	// Open file for writing (binary mode)
	fp = fopen(cFilename, "wb");
	if (fp == NULL) {
		errorMessage = "Could not open file " + String(filename) + " for writing";
		Godot::print("GDKRAPSDImporter Error: " + errorMessage);
		success = false;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		errorMessage = "Could not allocate write struct";
		Godot::print("GDKRAPSDImporter Error: " + errorMessage);
		success = false;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		errorMessage = "Could not allocate info struct";
		Godot::print("GDKRAPSDImporter Error: " + errorMessage);
		success = false;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		errorMessage = "Error during png creation";
		Godot::print("GDKRAPSDImporter Error: " + errorMessage);
		success = false;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	unsigned int channelCount = 0u;
	int colorType = 0;
	switch (channelType)
	{
	case MONOCHROME:
		channelCount = 1;
		colorType = PNG_COLOR_TYPE_GRAY;
		break;
	case RGB:
		channelCount = 3;
		colorType = PNG_COLOR_TYPE_RGB;
		break;
	case RGBA:
		channelCount = 4;
		colorType = PNG_COLOR_TYPE_RGBA;
		break;
	default:
		return false;
		break;
	}

	/* Write header depending on the channel type, always in 8 bit colour depth. */
	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, colorType, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	/* Allocate memory for one row (channelCount per pixel - RGB ) */
	row = (png_bytep) malloc(channelCount * width * sizeof(png_byte));

	/* Write image data, one row at a time. */
	unsigned int x, y;
	for (y=0 ; y<height ; y++) {
		for (x=0 ; x<width ; x++) {
			memcpy(row, &data[channelCount*width*y], channelCount * width * sizeof(png_byte));
		}
		png_write_row(png_ptr, row);
	}

	/* End the PNG write operation */
	png_write_end(png_ptr, NULL);

	/* Clear up the memory from the heap */
	finalise:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) std::free(row);

	return success;
}