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

#include <tinyxml2/tinyxml2.h>
#include <zlib/contrib/minizip/unzip.h>
#include <regex>

KRA_NAMESPACE_BEGIN

std::unique_ptr<KraDocument> CreateKraDocument(const std::wstring& filename);
void DestroyKraDocument(std::unique_ptr<KraDocument> &document);

unsigned int ParseUIntAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName);
const char* ParseCharAttribute(const tinyxml2::XMLElement *xmlElement, const char *attributeName);
const wchar_t* ParseWCharAttribute(const tinyxml2::XMLElement* xmlElement, const char* attributeName);

std::vector<std::unique_ptr<KraLayer>> ParseLayers(tinyxml2::XMLElement* xmlElement);

std::vector<std::unique_ptr<KraTile>> ParseTiles(std::vector<unsigned char> layerContent);
unsigned int ParseHeaderElement(std::vector<unsigned char> layerContent, const std::string & elementName, unsigned int& currentIndex);
std::string GetHeaderElement(std::vector<unsigned char> layerContent, unsigned int& currentIndex);

int extractCurrentFileToVector(std::vector<unsigned char>& resultVector, unzFile& m_zf);
int lzff_decompress(const void* input, int length, void* output, int maxout);

KRA_NAMESPACE_END
