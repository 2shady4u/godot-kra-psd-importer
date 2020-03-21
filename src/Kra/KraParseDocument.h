// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

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

KraDocument* CreateKraDocument(const std::wstring& filename);
void DestroyKraDocument(KraDocument*& document);

unsigned int ParseUIntAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName);
const char* ParseCharAttribute(const tinyxml2::XMLElement *xmlElement, const char *attributeName);
const wchar_t* ParseWCharAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName);

std::vector<KraLayer*> ParseLayers(tinyxml2::XMLElement* xmlElement);

std::vector<KraTile*> ParseTiles(std::vector<unsigned char> layerContent);
unsigned int ParseHeaderElement(std::vector<unsigned char> layerContent, const std::string & elementName, unsigned int& currentIndex);
std::string GetHeaderElement(std::vector<unsigned char> layerContent, unsigned int& currentIndex);

int lzff_decompress(const void* input, int length, void* output, int maxout);

KRA_NAMESPACE_END
