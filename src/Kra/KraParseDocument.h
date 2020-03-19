#pragma once

#include "KraPch.h"
#include "KraDocument.h"
#include "KraLayer.h"
#include "KraTile.h"

#include <zipper.h>
#include <unzipper.h>
#include <tinyxml2/tinyxml2.h>
#include <regex>

KRA_NAMESPACE_BEGIN

struct KraDocument;

/// \ingroup Parser
/// Parses the entire KRA document and creates a vector of layers.
KraDocument ParseKraDocument(const std::string& filename);

unsigned int ParseIntAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName);
const char* ParseCharAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName);
std::vector<KraLayer> ParseLayers(tinyxml2::XMLElement* xmlElement);
std::vector<KraTile> ParseTiles(std::vector<unsigned char> layerContent);
unsigned int ParseHeaderElement(std::vector<unsigned char> layerContent, const std::string & elementName, unsigned int& currentIndex);
std::string GetHeaderElement(std::vector<unsigned char> layerContent, unsigned int& currentIndex);

int lzff_decompress(const void* input, int length, void* output, int maxout);

KRA_NAMESPACE_END
