#pragma once

#include "KraPch.h"
#include "KraTile.h"
#include <vector>

KRA_NAMESPACE_BEGIN

/// \ingroup Types
/// \class Document
/// \brief A struct storing the document-wide information of a .KRA file.
/// \sa Section
struct KraLayer
{
    unsigned int colorSpaceName;			    ///< The number of channels stored in the document, including any additional alpha channels.
    unsigned int positionX;
    unsigned int positionY;
    unsigned int nodeType; //paintlayer
    unsigned int opacity;
    bool visible;
    const char* name;
    const char* fileName;

    std::vector<KraTile> tiles;
};

KRA_NAMESPACE_END