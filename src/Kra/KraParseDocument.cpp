// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#include "KraParseDocument.h"

KRA_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// Create a KRA Document which contains document properties and a vector of KraLayer-pointers.
// ---------------------------------------------------------------------------------------------------------------------
KraDocument *CreateKraDocument(const std::wstring &filename)
{

	/* Convert wstring to string */
	std::string sFilename(filename.begin(), filename.end());
	const char * path = sFilename.c_str();

	/* Open the KRA archive using minizip */
	unzFile m_zf = unzOpen(path);
	if (m_zf == NULL)
	{
		printf("(Parsing Document) ERROR: Failed to open KRA archive.\n");
		return NULL;
	}

	/* A 'maindoc.xml' file should always be present in the KRA archive, otherwise abort immediately */
	int errorCode = unzLocateFile(m_zf, "maindoc.xml", 1);
	if (errorCode == UNZ_OK)
	{
		printf("(Parsing Document) Found 'maindoc.xml', extracting document and layer properties\n");
	}
	else 
	{
		printf("(Parsing Document) WARNING: KRA archive did not contain maindoc.xml!\n");
		return NULL;
	}

	std::vector<unsigned char> resultVector;
	extractCurrentFileToVector(resultVector, m_zf);
	/* Put the vector into a string and parse it using tinyXML2 */
	std::string xmlString(resultVector.begin(), resultVector.end());
	tinyxml2::XMLDocument xmlDocument;
	xmlDocument.Parse(xmlString.c_str());
	tinyxml2::XMLElement *xmlElement = xmlDocument.FirstChildElement("DOC")->FirstChildElement("IMAGE");

    KraDocument *document = new KraDocument;
	/* Get important document attributes from the XML-file */
	document->width = ParseUIntAttribute(xmlElement, "width");
	document->height = ParseUIntAttribute(xmlElement, "height");
	document->name = ParseCharAttribute(xmlElement, "name");
	const char *colorSpaceName = ParseCharAttribute(xmlElement, "colorspacename");
	/* The color space defines the number of 'channels' */
	/* Each separate layer has its own color space in KRA, so not sure if this necessary... */
	if (strcmp(colorSpaceName, "RGBA") == 0)
	{
		document->channelCount = 4u;
	}
	else if (strcmp(colorSpaceName, "RGB") == 0)
	{
		document->channelCount = 3u;
	}
	else
	{
		document->channelCount = 0u;
	}
	printf("(Parsing Document) Document properties are extracted and have following values:\n");
	printf("(Parsing Document)  	>> width = %i\n", document->width);
	printf("(Parsing Document)  	>> height = %i\n", document->height);
	printf("(Parsing Document)  	>> name = %s\n", document->name);
	printf("(Parsing Document)  	>> channelCount = %i\n", document->channelCount);

	/* Parse all the layers registered in the maindoc.xml and add them to the document */
	document->layers = ParseLayers(xmlElement);

	/* Go through all the layers and initiate their tiles */
	/* Only layers of type paintlayer get  their tiles parsed8 */
	/* This also automatically de-encrypts the tile data */
	for (auto &layer : document->layers)
	{
		if (layer->type == kraLayerType::PAINT_LAYER)
		{
			const std::string &layerPath = (std::string)document->name + "/layers/" + (std::string)layer->filename;
			std::vector<unsigned char> layerContent;
			const char* cLayerPath = layerPath.c_str(); 
			int errorCode = unzLocateFile(m_zf, cLayerPath, 1);
			errorCode += extractCurrentFileToVector(layerContent, m_zf);
			if (errorCode == UNZ_OK)
			{
				/* Start extracting the tile data. */
				layer->tiles = ParseTiles(layerContent);
			}
			else
			{
				printf("(Parsing Document) WARNING: Layer entry with path '%s' could not be found in KRA archive.\n", layerPath.c_str());
			}
		}
	}

	errorCode = unzClose(m_zf);

	return document;

}

// ---------------------------------------------------------------------------------------------------------------------
// Cleans up all allocated memory on the heap.
// ---------------------------------------------------------------------------------------------------------------------
void DestroyKraDocument(KraDocument *&document)
{
	while (document->layers.size() > 0)
	{
		KraLayer *layer = document->layers.back();
		while (layer->tiles.size() > 0)
		{
			KraTile *tile = layer->tiles.back();
			layer->tiles.pop_back();
			free(tile->data);
			delete tile;
		}
		document->layers.pop_back();
		delete layer;
	}
	delete document;
}

// ---------------------------------------------------------------------------------------------------------------------
// Parses a XMLElement and extracts the UNSIGNED INT attribute value found at the attribute name.
// ---------------------------------------------------------------------------------------------------------------------
unsigned int ParseUIntAttribute(const tinyxml2::XMLElement *xmlElement, const char *attributeName)
{
	unsigned int attributeValue = -1;
	const tinyxml2::XMLAttribute *attribute = xmlElement->FindAttribute(attributeName);
	if (attribute == 0)
	{
		printf("(Parsing Document) WARNING: Missing XML attribute '%s'\n", attributeName);
	}
	else
	{
		const char *charValue = attribute->Value();
		std::stringstream strValue;
		strValue << charValue;
		strValue >> attributeValue;
	}
	return attributeValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// Parses a XMLElement and extracts the CONST CHAR* attribute value found at the attribute name.
// ---------------------------------------------------------------------------------------------------------------------
const char *ParseCharAttribute(const tinyxml2::XMLElement *xmlElement, const char *attributeName)
{
	const char *attributeValue;
	const tinyxml2::XMLAttribute *attribute = xmlElement->FindAttribute(attributeName);
	if (attribute == 0)
	{
		printf("(Parsing Document) WARNING: Missing XML attribute '%s'\n", attributeName);
	}
	else
	{
		attributeValue = attribute->Value();
	}
	return attributeValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// Parses a XMLElement and extracts the CONST WCHAR_T* attribute value found at the attribute name.
// ---------------------------------------------------------------------------------------------------------------------
const wchar_t *ParseWCharAttribute(const tinyxml2::XMLElement *xmlElement, const char *attributeName)
{
	const wchar_t *attributeValue;
	const tinyxml2::XMLAttribute *attribute = xmlElement->FindAttribute(attributeName);
	if (attribute == 0)
	{
		printf("(Parsing Document) WARNING: Missing XML attribute '%s'\n", attributeName);
	}
	else
	{
		const char *attributeChar = attribute->Value();
		const size_t cSize = strlen(attributeChar) + 1;
		wchar_t *attributeWChar = new wchar_t[cSize];
		mbstowcs(attributeWChar, attributeChar, cSize);

		attributeValue = attributeWChar;
	}
	return attributeValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// Go through the XML-file and extract all the layer properties.
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraLayer *> ParseLayers(tinyxml2::XMLElement *xmlElement)
{
	std::vector<KraLayer *> layers;
	const tinyxml2::XMLElement *layersElement = xmlElement->FirstChildElement("layers");
	const tinyxml2::XMLElement *layerNode = layersElement->FirstChild()->ToElement();

	/* Hopefully we find something... otherwise there are no layers! */
	/* Keep trying to find a layer until we can't any new ones! */
	while (layerNode != 0)
	{
		KraLayer *layer = new KraLayer;
		/* Get important layer attributes from the XML-file */
		layer->x = ParseUIntAttribute(layerNode, "x");
		layer->y = ParseUIntAttribute(layerNode, "y");
		layer->opacity = ParseUIntAttribute(layerNode, "opacity");
		layer->isVisible = ParseUIntAttribute(layerNode, "visible");

		layer->name = ParseWCharAttribute(layerNode, "name");
		layer->filename = ParseCharAttribute(layerNode, "filename");
		const char *colorSpaceName = ParseCharAttribute(layerNode, "colorspacename");
		/* The color space defines the number of 'channels' */
		/* Each seperate layer has its own color space in KRA, so not sure if this necessary... */
		if (strcmp(colorSpaceName, "RGBA") == 0)
		{
			layer->channelCount = 4u;
		}
		else if (strcmp(colorSpaceName, "RGB") == 0)
		{
			layer->channelCount = 3u;
		}
		else
		{
			layer->channelCount = 0u;
		}

		const char *nodeType = ParseCharAttribute(layerNode, "nodetype");
		/* The color space defines the number of 'channels' */
		/* Each seperate layer has its own color space in KRA, so not sure if this necessary... */
		if (strcmp(nodeType, "paintlayer") == 0)
		{
			layer->type = kraLayerType::PAINT_LAYER;
		}
		else if (strcmp(nodeType, "vectorlayer") == 0)
		{
			layer->type = kraLayerType::VECTOR_LAYER;
		}
		else if (strcmp(nodeType, "grouplayer") == 0)
		{
			layer->type = kraLayerType::GROUP_LAYER;
		}
		else
		{
			layer->type = kraLayerType::OTHER;
		}

		std::wstring ws(layer->name);
		std::string str(ws.begin(), ws.end());
		const char * cname = str.c_str();

		printf("(Parsing Document) Layer '%s' properties are extracted and have following values:\n", cname);
		printf("(Parsing Document)  	>> name = %s\n", cname);
		printf("(Parsing Document)  	>> filename = %s\n", layer->filename);
		printf("(Parsing Document)  	>> channelCount = %i\n", layer->channelCount);
		printf("(Parsing Document)  	>> opacity = %i\n", layer->opacity);
		printf("(Parsing Document)  	>> type = %i\n", layer->type);
		printf("(Parsing Document)  	>> isVisible = %s\n", layer->isVisible ? "true" : "false");
		printf("(Parsing Document)  	>> x = %i\n", layer->x);
		printf("(Parsing Document)  	>> y = %i\n", layer->y);

		layers.push_back(layer);

		/* Try to get the next layer entry... if not available break */
		const tinyxml2::XMLNode *nextSibling = layerNode->NextSibling();
		if (nextSibling == 0)
		{
			break;
		}
		else
		{
			layerNode = nextSibling->ToElement();
		}
	}

	return layers;
}

// ---------------------------------------------------------------------------------------------------------------------
// Extract the tile data and properties from the raw binary data.
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraTile *> ParseTiles(std::vector<unsigned char> layerContent)
{

	std::vector<KraTile *> tiles;

	/* This code works with a global pointer index that gets incremented depending on element size */
	/* currentIndex obviously starts at zero and will be passed by reference */
	unsigned int currentIndex = 0;

	/* Extract the main header from the tiles */
	unsigned int version = ParseHeaderElement(layerContent, "VERSION ", currentIndex);
	unsigned int tileWidth = ParseHeaderElement(layerContent, "TILEWIDTH ", currentIndex);
	unsigned int tileHeight = ParseHeaderElement(layerContent, "TILEHEIGHT ", currentIndex);
	unsigned int pixelSize = ParseHeaderElement(layerContent, "PIXELSIZE ", currentIndex);
	unsigned int decompressedLength = pixelSize * tileWidth * tileHeight;

	printf("(Parsing Document) Tile properties (Main Header) are extracted and have following values:\n");
	printf("(Parsing Document)  	>> version = %i\n", version);
	printf("(Parsing Document)  	>> tileWidth = %i\n", tileWidth);
	printf("(Parsing Document)  	>> tileHeight = %i\n", tileHeight);
	printf("(Parsing Document)  	>> pixelSize = %i\n", pixelSize);
	printf("(Parsing Document)  	>> decompressedLength = %i\n", decompressedLength);

	unsigned int numberOfTiles = ParseHeaderElement(layerContent, "DATA ", currentIndex);

	for (unsigned int i = 0; i < numberOfTiles; i++)
	{
		KraTile *tile = new KraTile;
		tile->version = version;
		tile->tileWidth = tileWidth;
		tile->tileHeight = tileHeight;
		tile->pixelSize = pixelSize;
		tile->decompressedLength = decompressedLength;

		/* Now it is time to extract & decompress the data */
		/* First the non-general element of the header needs to be extracted */
		std::string headerString = GetHeaderElement(layerContent, currentIndex);
		std::regex e("(-?\\d*),(-?\\d*),(\\w*),(\\d*)");
		std::smatch sm;
		std::regex_match(headerString, sm, e);

		/* This header contains: */
		/* 1. A number that defines the left position of the tile (CAN BE NEGATIVE!!!) */
		/* 2. A number that defines the top position of the tile (CAN BE NEGATIVE!!!) */
		/* 3. The string "LZF" which states the compression algorithm */
		/* 4. A number that defines the number of bytes of the data that comes after this header. */

		/* sm[0] is the full string match which we don't need */
		/* The left position of the tile */
		std::stringstream strValue;
		strValue << sm[1];
		strValue >> tile->left;
		/* The top position of the tile */
		strValue.clear();
		strValue << sm[2];
		strValue >> tile->top;
		/* We don't really care about sm[3] since it is always 'LZF' */
		/* The number of compressed bytes coming after this header */
		strValue.clear();
		strValue << sm[4];
		strValue >> tile->compressedLength;

		/* Put all the data in a vector */
		std::vector<unsigned char> dataVector(layerContent.begin() + currentIndex, layerContent.begin() + currentIndex + tile->compressedLength);
		const uint8_t *dataVectorPointer = dataVector.data();
		/* Allocate memory for the output */
		uint8_t *output = (uint8_t *)malloc(tile->decompressedLength);

		/* Now... the first byte of this dataVector is actually an indicator of compression */
		/* As follows: */
		/* 0 -> No compression, the data is actually raw! */
		/* 1 -> The data was compressed using LZF */
		/* TODO: Actually implement a check to see this byte!!! */
		lzff_decompress(dataVectorPointer + 1, tile->compressedLength, output, tile->decompressedLength);

		/* TODO: Krita might also normalize the colors in some way */
		/* This needs to be check and if present, the colors need to be denormalized */

		/* Data is saved in following format: */
		/* - Firstly all the RED data */
		/* - Secondly all the GREEN data */
		/* - Thirdly all the BLUE data */
		/* - Fourthly all the ALPHA data */
		/* This is different from the required ImageMagick format which requires quartets of: */
		/* R1, G1, B1, A1, R2, G2, ... etc */
		/* We'll just sort this here!*/
		/* TODO: Sometimes there won't be any alpha channel when it is RGB instead of RGBA. */
		uint8_t *sortedOutput = (uint8_t *)malloc(tile->decompressedLength);
		int jj = 0;
		int tileArea = tile->tileHeight * tile->tileWidth;
		for (int i = 0; i < tileArea; i++)
		{
			sortedOutput[jj] = output[2 * tileArea + i];	 //RED CHANNEL
			sortedOutput[jj + 1] = output[tileArea + i];	 //GREEN CHANNEL
			sortedOutput[jj + 2] = output[i];				 //BLUE CHANNEL
			sortedOutput[jj + 3] = output[3 * tileArea + i]; //ALPHA CHANNEL
			jj = jj + 4;
		}
		tile->data = sortedOutput;
		/* Q: Why are the RED and BLUE channels swapped? */
		/* A: I have no clue... that's how it is saved in the tile! */

		printf("(Parsing Document) Tile properties (Tile Header) are extracted and have following values:\n");
		printf("(Parsing Document)  	>> left = %i\n", tile->left);
		printf("(Parsing Document)  	>> top = %i\n", tile->top);
		printf("(Parsing Document)  	>> compressedLength = %i\n", tile->compressedLength);
		printf("(Parsing Document)  	>> DATA START\n");
		int j = 0;
		for (int i = 0; i < 256; i++)
		{
			j++;
			printf("%i ", sortedOutput[i]);
			if (j == 64)
			{
				j = 0;
				printf("\n");
			}
		}
		printf("(Parsing Document)  	>> DATA END\n");

		tiles.push_back(tile);

		/* Add the compressedLength to the currentIndex so the next tile starts at the correct position */
		currentIndex += tile->compressedLength;
	}

	return tiles;
}

// ---------------------------------------------------------------------------------------------------------------------
// Extract a header element and match it with the element name.
// ---------------------------------------------------------------------------------------------------------------------
unsigned int ParseHeaderElement(std::vector<unsigned char> layerContent, const std::string &elementName, unsigned int &currentIndex)
{
	unsigned int elementIntValue = -1;
	/* First extract the header element */
	std::string elementValue = GetHeaderElement(layerContent, currentIndex);
	/* Try to match the elementValue string */
	if (elementValue.find(elementName) != std::string::npos)
	{
		size_t pos = elementValue.find(elementName);
		/* If found then erase it from string */
		elementValue.erase(pos, elementName.length());
		/* Dump it into the output variable using stringstream */
		std::stringstream strValue;
		strValue << elementValue;
		strValue >> elementIntValue;
	}
	else
	{
		printf("(Parsing Document) WARNING: Missing header element in tile with name '%s'\n", elementName.c_str());
	}
	return elementIntValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// Extract a header element starting from the currentIndex until the next "0x0A".
// ---------------------------------------------------------------------------------------------------------------------
std::string GetHeaderElement(std::vector<unsigned char> layerContent, unsigned int &currentIndex)
{
	unsigned int startIndex = currentIndex;
	/* Just go through the vector until you encounter "0x0A" */
	while (layerContent.at(currentIndex) != (char)0x0A)
	{
		currentIndex++;
	}
	unsigned int endIndex = currentIndex;
	/* Extract this header element */
	std::vector<unsigned char> elementContent(layerContent.begin() + startIndex, layerContent.begin() + endIndex);
	/* Print it... should be disabled at some point */
	for (std::vector<int>::size_type i = 0; i < elementContent.size(); i++)
	{
		std::cout << elementContent.at(i) << ' ';
	}
	std::cout << "\n";
	std::string elementValue(reinterpret_cast<char *>(elementContent.data()), endIndex - startIndex);
	/* Increment the currentIndex pointer so that we skip the "0x0A" character */
	currentIndex++;

	return elementValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// Extra the data content of the current file in the ZIP archive to a vector.
// ---------------------------------------------------------------------------------------------------------------------

int extractCurrentFileToVector(std::vector<unsigned char>& resultVector, unzFile& m_zf)
{
	size_t errorCode = UNZ_ERRNO;
 	unz_file_info64 file_info = { 0 };
    char filename_inzip[256] = { 0 };

	/* Get the required size necessary to store the file content. */
    errorCode = unzGetCurrentFileInfo64(m_zf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
	size_t uncompressed_size = file_info.uncompressed_size;

	errorCode = unzOpenCurrentFile(m_zf);

	std::vector<unsigned char> buffer;
	buffer.resize(WRITEBUFFERSIZE);
	resultVector.reserve((size_t)file_info.uncompressed_size);

	/* error_code serves also as the number of bytes that were read... */
	do
	{
	/* Read the data in parts of size WRITEBUFFERSIZE */
	/* and keep reading until the function returns zero or lower. */
	errorCode = unzReadCurrentFile(m_zf, buffer.data(), (unsigned int)buffer.size());
	if (errorCode < 0 || errorCode == 0)
		break;

	resultVector.insert(resultVector.end(), buffer.data(), buffer.data() + errorCode);

	} while (errorCode > 0);

	/* Be sure to close the file to avoid leakage. */
	errorCode = unzCloseCurrentFile(m_zf);

	return (int)errorCode;
}


// ---------------------------------------------------------------------------------------------------------------------
// Decompression function for LZF copied directly (with minor modifications) from the Krita codebase (libs\image\tiles3\swap\kis_lzf_compression.cpp).
// ---------------------------------------------------------------------------------------------------------------------

int lzff_decompress(const void *input, int length, void *output, int maxout)
{
	const unsigned char *ip = (const unsigned char *)input;
	const unsigned char *ip_limit = ip + length - 1;
	unsigned char *op = (unsigned char *)output;
	unsigned char *op_limit = op + maxout;
	unsigned char *ref;

	while (ip < ip_limit)
	{
		unsigned int ctrl = (*ip) + 1;
		unsigned int ofs = ((*ip) & 31) << 8;
		unsigned int len = (*ip++) >> 5;

		if (ctrl < 33)
		{
			/* literal copy */
			if (op + ctrl > op_limit)
				return 0;

			/* crazy unrolling */
			if (ctrl)
			{
				*op++ = *ip++;
				ctrl--;

				if (ctrl)
				{
					*op++ = *ip++;
					ctrl--;

					if (ctrl)
					{
						*op++ = *ip++;
						ctrl--;

						for (; ctrl; ctrl--)
							*op++ = *ip++;
					}
				}
			}
		}
		else
		{
			/* back reference */
			len--;
			ref = op - ofs;
			ref--;

			if (len == 7 - 1)
				len += *ip++;

			ref -= *ip++;

			if (op + len + 3 > op_limit)
				return 0;

			if (ref < (unsigned char *)output)
				return 0;

			*op++ = *ref++;
			*op++ = *ref++;
			*op++ = *ref++;
			if (len)
				for (; len; --len)
					*op++ = *ref++;
		}
	}

	return (int)(op - (unsigned char *)output);
}

KRA_NAMESPACE_END
