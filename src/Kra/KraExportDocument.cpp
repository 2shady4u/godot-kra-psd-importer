#include "KraExportDocument.h"
#include <iostream>

KRA_NAMESPACE_BEGIN

struct KraExtents
{
    int min_x, max_x, min_y, max_y = 0;

};

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraExportedLayer> ExportKraDocument(KraDocument document)
{

    std::vector<KraExportedLayer> exportedLayers;
    for (auto const& layer : document.layers){

        KraExportedLayer exportedLayer = {};
        size_t composedDataSize = 0;

        KraExtents extents = {};
        // find the extents of the layer canvas.
        for (auto const& tile : layer.tiles)
        {
            if (tile.offsetX < extents.min_x)
            {
                extents.min_x = tile.offsetX;
            }
            if (tile.offsetX + (int)tile.tileWidth > extents.max_x)
            {
                extents.max_x = tile.offsetX + (int)tile.tileWidth;
            }

            if (tile.offsetY < extents.min_y)
            {
                extents.min_y = tile.offsetY;
            }
            if (tile.offsetY + (int)tile.tileHeight > extents.max_y)
            {
                extents.max_y = tile.offsetY + (int)tile.tileHeight;
            }

        }
        exportedLayer.layerWidth = extents.max_x - extents.min_x;
        exportedLayer.layerHeight = extents.max_y - extents.min_y;

        KraTile referenceTile = layer.tiles[0];
        unsigned int numberOfColumns = (unsigned int)exportedLayer.layerWidth/referenceTile.tileWidth;
        unsigned int numberOfRows = (unsigned int)exportedLayer.layerHeight/referenceTile.tileHeight;
        composedDataSize = numberOfColumns*numberOfRows*referenceTile.decompressedLength;

        uint8_t* composedData = (uint8_t*) malloc(composedDataSize);

        for (auto const& tile : layer.tiles)
        {
            int currentOffsetY = tile.offsetY - extents.min_y;
            int currentOffsetX = tile.offsetX - extents.min_x;
            for (int rowIndex = 0; rowIndex < (int)tile.tileHeight; rowIndex++)
            {
                uint8_t* destination = composedData + tile.pixelSize*tile.tileWidth*rowIndex*numberOfColumns;
                destination += tile.pixelSize*currentOffsetX;
                destination += tile.pixelSize*tile.tileWidth*currentOffsetY*numberOfColumns;
                uint8_t* source = tile.data + tile.pixelSize*tile.tileWidth*rowIndex;
                size_t size = tile.pixelSize*tile.tileWidth;
                memcpy(destination, source, size);
            }
        }
        exportedLayer.data = composedData;

        exportedLayers.push_back(exportedLayer);

    }
    return exportedLayers;

}

KRA_NAMESPACE_END