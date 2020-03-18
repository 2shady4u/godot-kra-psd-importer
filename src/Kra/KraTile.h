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
    unsigned int height;
    unsigned int width;
    unsigned int pixelSize;

    unsigned int positionX;
    unsigned int positionY;

    int compressedLength;
    int decompressedLength;

    unsigned char* data;
};

KRA_NAMESPACE_END