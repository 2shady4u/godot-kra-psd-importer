#pragma once

#include "KraPch.h"

KRA_NAMESPACE_BEGIN

/// \ingroup Types
/// \class Document
/// \brief A struct storing the document-wide information of a .KRA file.
/// \sa Section
struct KraExportedLayer
{
    unsigned int colorSpaceName;			    ///< The number of channels stored in the document, including any additional alpha channels.
    unsigned int positionX;
    unsigned int positionY;

    unsigned int layerWidth = 0;
    unsigned int layerHeight = 0;

    unsigned int nodeType; //paintlayer
    unsigned int opacity;
    bool visible;
    const char* name;

    uint8_t* data;
};

KRA_NAMESPACE_END