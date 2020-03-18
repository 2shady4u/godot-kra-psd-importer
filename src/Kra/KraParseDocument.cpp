#include "KraParseDocument.h"

KRA_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
KraDocument ParseKraDocument(const std::string& filename)
{
	bool has_maindoc = false;
	KraDocument document = {};

    zipper::Unzipper unzipper(filename);
	std::vector<zipper::ZipEntry> entries = unzipper.entries();
	for(auto const& value: entries) {
		if (value.name == "maindoc.xml")
		{
			std::cout << value.name << "\n";
			has_maindoc = true;

			std::vector<unsigned char> resvec;
			unzipper.extractEntryToMemory("maindoc.xml", resvec);
			std::string xmlString(resvec.begin(), resvec.end());
			tinyxml2::XMLDocument xmlDocument;
			xmlDocument.Parse(xmlString.c_str());
			tinyxml2::XMLElement* xmlElement = xmlDocument.FirstChildElement( "DOC" )->FirstChildElement( "IMAGE" );

			//const std::vector<const char*> attributeNames{ "width", "height", "colorspacename"};
			
			document.width = ParseIntAttribute(xmlElement, "width");
			document.height = ParseIntAttribute(xmlElement, "height");
			document.name = ParseCharAttribute(xmlElement, "name");

			//Parse the layers and add them to the document?
			document.layers = ParseLayers(xmlElement);

			//Go through all the layers and initiate their tiles.
			for (auto& layer : document.layers)
			{
				const std::string& layerPath = (std::string)document.name + "/layers/" + (std::string)layer.fileName;
				std::vector<unsigned char> layerContent;
				bool success = unzipper.extractEntryToMemory(layerPath, layerContent);
				std::cout << "path was: " << layerPath << ", success was: " << success << "\n";

				layer.tiles = ParseTiles(layerContent);
			}

			break;
		}
	}

	if (has_maindoc == false)
	{
		printf("KRA file did not contain maindoc.xml!");
	}
	unzipper.close();

	return document;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
unsigned int ParseIntAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName)
{
	unsigned int attributeValue = -1;
	const tinyxml2::XMLAttribute* attribute = xmlElement->FindAttribute(attributeName);
	if (attribute == 0)
	{
		printf("Missing attribute in maindoc.xml '%s'\n", attributeName);
	} 
	else
	{
		const char* charValue = attribute->Value();
		std::stringstream strValue;
		strValue << charValue;
		strValue >> attributeValue;
	}
	return attributeValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
const char* ParseCharAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName)
{
	const char* attributeValue;
	const tinyxml2::XMLAttribute* attribute = xmlElement->FindAttribute(attributeName);
	if (attribute == 0)
	{
		printf("Missing attribute in maindoc.xml '%s'\n", attributeName);
	} 
	else
	{	
		attributeValue = attribute->Value();
	}
	return attributeValue;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraLayer> ParseLayers(tinyxml2::XMLElement* xmlElement)
{
	std::vector<KraLayer> layers;
	const tinyxml2::XMLElement* layersElement = xmlElement->FirstChildElement( "layers" );
	const tinyxml2::XMLElement* layerNode = layersElement->FirstChild()->ToElement();
	std::cout << "pup" << "\n";
	while (layerNode != 0)
	{
		std::cout << "plre" << "\n";
		KraLayer layer = {};
		layer.positionX = ParseIntAttribute(layerNode, "x");
		layer.positionY = ParseIntAttribute(layerNode, "y");
		layer.name = ParseCharAttribute(layerNode, "name");
		layer.fileName = ParseCharAttribute(layerNode, "filename");
		layers.push_back(layer);

		const tinyxml2::XMLNode* nextSibling = layerNode->NextSibling();
		if (nextSibling == 0)
		{
			break;
		}
		else
		{
			layerNode = nextSibling->ToElement();
		}
	}
	std::cout << "plop" << "\n";

	return layers;
}

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraTile> ParseTiles(std::vector<unsigned char> layerContent)
{

	std::vector<KraTile> tiles;
	std::cout << "size " << layerContent.size() << "\n";

	unsigned int currentIndex = 0;
	unsigned int start_index = 0;
	unsigned int end_index = 0;
	//There's at least one tile?
	unsigned int numberOfTiles = 1;
	KraTile tile = {};

	const std::string & lzfString = "0,0,LZF,";

	tile.version = ParseHeaderElement(layerContent, "VERSION ", currentIndex);
	tile.width = ParseHeaderElement(layerContent, "TILEWIDTH ", currentIndex);
	tile.height = ParseHeaderElement(layerContent, "TILEHEIGHT ", currentIndex);
	tile.pixelSize = ParseHeaderElement(layerContent, "PIXELSIZE ", currentIndex);
	tile.decompressedLength = tile.pixelSize*tile.width*tile.height;

	numberOfTiles = ParseHeaderElement(layerContent, "DATA ", currentIndex);
	for (unsigned int i = 0; i < numberOfTiles; i++) 
	{
		KraTile copiedTile = tile;

		//Now it is time to extract & decompress the data!
		//First get the last header element out of there!
		std::string headerString = GetHeaderElement(layerContent, currentIndex);
		std::string delimiter = ",";

		size_t pos = 0;
		std::string token;
		int currentToken = 0;
		while ((pos = headerString.find(delimiter)) != std::string::npos)
		{
    		token = headerString.substr(0, pos);
			std::stringstream strValue;
			strValue << token;
			switch (currentToken)
			{
			case 0:
				strValue >> copiedTile.positionX;
				break;
			case 1:
				strValue >> copiedTile.positionY;
				break;
			case 2: // just LZF, don't care about this!
				break;
			case 3:
				strValue >> copiedTile.compressedLength;
				break;
			default:
				break;
			}
    		headerString.erase(0, pos + delimiter.length());
		}

		std::cout << copiedTile.compressedLength << "\n";

		std::vector<unsigned char> clippedContent(layerContent.begin()+currentIndex, layerContent.begin()+currentIndex+tile.compressedLength);
		const uint8_t* clippedContent_ptr = clippedContent.data();

		uint8_t* output = (uint8_t*) malloc(tile.decompressedLength);
		lzff_decompress(clippedContent_ptr + 1, tile.compressedLength, output, tile.decompressedLength);
		uint8_t* sortedOutput = (uint8_t*) malloc(tile.decompressedLength);
		int jj = 0;
		for (int i = 0; i < 64*64; i++){
				sortedOutput[jj] = output[2*64*64+i]; //RED CHANNEL
				sortedOutput[jj+1] = output[64*64+i]; //GREEN CHANNEL
				sortedOutput[jj+2] = output[i]; //BLUE CHANNEL
				sortedOutput[jj+3] = output[3*64*64+i]; //ALPHA CHANNEL
				jj = jj + 4;
		}
		tile.data = sortedOutput;

		tiles.push_back(copiedTile);

		currentIndex += tile.compressedLength;

	}

	return tiles;
}

unsigned int ParseHeaderElement(std::vector<unsigned char> layerContent, const std::string & elementName, unsigned int& currentIndex)
{
	std::cout << "currently at " << currentIndex << "\n";
	unsigned int elementIntValue = -1;
	unsigned int startIndex = currentIndex;
	while (layerContent.at(currentIndex) != (char)0x0A)
	{
		currentIndex++;
	}
	unsigned int endIndex = currentIndex;
	std::vector<unsigned char> elementContent(layerContent.begin() + startIndex, layerContent.begin() + endIndex);
	for (std::vector<int>::size_type i = 0; i < elementContent.size(); i++) 
	{
		std::cout << elementContent.at(i) << ' ';
	}
	std::cout << "\n";
	std::string elementValue(reinterpret_cast<char*>(elementContent.data()), endIndex - startIndex);
	if (elementValue.find(elementName) != std::string::npos)
	{
		size_t pos = elementValue.find(elementName);
		// If found then erase it from string
		elementValue.erase(pos, elementName.length());
		std::cout << elementName << " is " << elementValue << "\n";
		std::stringstream strValue;
		strValue << elementValue;
		strValue >> elementIntValue;
		
	} 
	else 
	{
		std::cout << "'" << elementName << "' could not be found!" << "\n";
	}
	// Skip the 0x0A
	currentIndex++;
	return elementIntValue;
}

std::string GetHeaderElement(std::vector<unsigned char> layerContent, unsigned int& currentIndex)
{
	std::cout << "currently at " << currentIndex << "\n";
	unsigned int elementIntValue = -1;
	unsigned int startIndex = currentIndex;
	while (layerContent.at(currentIndex) != (char)0x0A)
	{
		currentIndex++;
	}
	unsigned int endIndex = currentIndex;
	std::vector<unsigned char> elementContent(layerContent.begin() + startIndex, layerContent.begin() + endIndex);
	for (std::vector<int>::size_type i = 0; i < elementContent.size(); i++) 
	{
		std::cout << elementContent.at(i) << ' ';
	}
	std::cout << "\n";
	std::string elementValue(reinterpret_cast<char*>(elementContent.data()), endIndex - startIndex);
	return elementValue;
}

int lzff_decompress(const void* input, int length, void* output, int maxout)
{
    const unsigned char* ip = (const unsigned char*) input;
    const unsigned char* ip_limit  = ip + length - 1;
    unsigned char* op = (unsigned char*) output;
    unsigned char* op_limit = op + maxout;
    unsigned char* ref;

    while (ip < ip_limit) {
        unsigned int ctrl = (*ip) + 1;
        unsigned int ofs = ((*ip) & 31) << 8;
        unsigned int len = (*ip++) >> 5;

        if (ctrl < 33) {
            /* literal copy */
            if (op + ctrl > op_limit)
                return 0;

            /* crazy unrolling */
            if (ctrl) {
                *op++ = *ip++;
                ctrl--;

                if (ctrl) {
                    *op++ = *ip++;
                    ctrl--;

                    if (ctrl) {
                        *op++ = *ip++;
                        ctrl--;

                        for (; ctrl; ctrl--)
                            *op++ = *ip++;
                    }
                }
            }
        } else {
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

    return op - (unsigned char*)output;
}

KRA_NAMESPACE_END
