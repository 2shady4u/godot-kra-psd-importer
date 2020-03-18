#include "KraParseDocument.h"

KRA_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraTile> ExportKraDocument(KraDocument document)
{

    std::vector<KraTile> exportedTiles;
    for (auto const& layer : document.layers){
        for (auto const& tile : layer.tiles)
        {
            exportedTiles.push_back(tile);
        }
    }
    return exportedTiles;

}

KRA_NAMESPACE_END