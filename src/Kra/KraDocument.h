#pragma once

#include "KraPch.h"
#include "KraLayer.h"
#include <vector>

KRA_NAMESPACE_BEGIN

/// \ingroup Types
/// \class Document
/// \brief A struct storing the document-wide information of a .KRA file.
/// \sa Section
struct KraDocument
{
    unsigned int width;							///< The width of the document.
    unsigned int height;						///< The height of the document.
    unsigned int colorSpace = COLOR_SPACE::RGBA;			    ///< The number of channels stored in the document, including any additional alpha channels.
    const char* name;

    std::vector<KraLayer> layers;
};

KRA_NAMESPACE_END