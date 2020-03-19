#pragma once

#include "KraPch.h"

KRA_NAMESPACE_BEGIN

/// \ingroup Types
/// \class Document
/// \brief A struct storing the document-wide information of a .KRA file.
/// \sa Section
struct KraTile
{

    unsigned int version;
    unsigned int tileHeight;
    unsigned int tileWidth;
    unsigned int pixelSize;

    // Offset can also be negative!
    int offsetX;
    int offsetY;

    int compressedLength;
    int decompressedLength;

    uint8_t* data;
};

KRA_NAMESPACE_END