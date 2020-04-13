// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#include "KraExport.h"

KRA_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// Take all the layers and their tiles and construct/compose the complete image!
// ---------------------------------------------------------------------------------------------------------------------
std::vector<KraExportedLayer *> CreateKraExportLayers(KraDocument *document)
{

    std::vector<KraExportedLayer *> exportedLayers;
    /* Go through all the layers and add them to the exportedLayers vector */
    for (auto const &layer : document->layers)
    {
        if (layer->type != kraLayerType::PAINT_LAYER)
        {
            std::wstring ws(layer->name);
		    std::string str(ws.begin(), ws.end());
		    const char * cname = str.c_str();
            printf("(Exporting Document) Ignoring non-exportable Layer '%s'\n", cname);
        }
        else
        {
            KraExportedLayer *exportedLayer = new KraExportedLayer;
            /* Copy all important properties immediately */
            exportedLayer->name = layer->name;
            exportedLayer->channelCount = layer->channelCount;
            exportedLayer->x = layer->x;
            exportedLayer->y = layer->y;
            exportedLayer->opacity = layer->opacity;
            exportedLayer->isVisible = layer->isVisible;

            /*Initialize the extents of this layer to 0 */
            exportedLayer->left = 0;
            exportedLayer->right = 0;
            exportedLayer->top = 0;
            exportedLayer->bottom = 0;

            /* find the extents of the layer canvas */
            for (auto const &tile : layer->tiles)
            {
                if (tile->left < exportedLayer->left)
                {
                    exportedLayer->left = tile->left;
                }
                if (tile->left + (int32_t)tile->tileWidth > exportedLayer->right)
                {
                    exportedLayer->right = tile->left + (int32_t)tile->tileWidth;
                }

                if (tile->top < exportedLayer->top)
                {
                    exportedLayer->top = tile->top;
                }
                if (tile->top + (int32_t)tile->tileHeight > exportedLayer->bottom)
                {
                    exportedLayer->bottom = tile->top + (int32_t)tile->tileHeight;
                }
            }
            unsigned int layerHeight = (unsigned int)(exportedLayer->bottom - exportedLayer->top);
            unsigned int layerWidth = (unsigned int)(exportedLayer->right - exportedLayer->left);

            /* Get a reference tile and extract the number of horizontal and vertical tiles */
            KraTile *referenceTile = layer->tiles[0];
            unsigned int numberOfColumns = layerWidth / referenceTile->tileWidth;
            unsigned int numberOfRows = layerHeight / referenceTile->tileHeight;
            size_t composedDataSize = numberOfColumns * numberOfRows * referenceTile->decompressedLength;

            std::wstring ws(exportedLayer->name);
		    std::string str(ws.begin(), ws.end());
		    const char * cname = str.c_str();

            printf("(Exporting Document) Exported Layer '%s' properties are extracted and have following values:\n", cname);
            printf("(Exporting Document)  	>> numberOfColumns = %i\n", numberOfColumns);
            printf("(Exporting Document)  	>> numberOfRows = %i\n", numberOfRows);
            printf("(Exporting Document)  	>> layerWidth = %i\n", layerWidth);
            printf("(Exporting Document)  	>> layerHeight = %i\n", layerHeight);
            printf("(Exporting Document)  	>> top = %i\n", exportedLayer->top);
            printf("(Exporting Document)  	>> bottom = %i\n", exportedLayer->bottom);
            printf("(Exporting Document)  	>> left = %i\n", exportedLayer->left);
            printf("(Exporting Document)  	>> right = %i\n", exportedLayer->right);
            printf("(Exporting Document)  	>> composedDataSize = %i\n", static_cast<int>(composedDataSize));

            /* Allocate space for the output data! */
            uint8_t *composedData = (uint8_t *)malloc(composedDataSize);
            /* I initialize all these elements to zero to avoid empty tiles from being filled with junk */
            /* Problem might be that this takes quite a lot of time... */
            /* TODO: Only the empty tiles should be initialized to zero! */
            std::memset(composedData, 0, composedDataSize);

            /* IMPORTANT: Not all the tiles exist! */
            /* Empty tiles (containing full ALPHA) are not added as tiles! */
            /* Now we have to construct the data in such a way that all tiles are in the correct positions */
            for (auto const &tile : layer->tiles)
            {
                int currentNormalizedTop = tile->top - exportedLayer->top;
                int currentNormalizedLeft = tile->left - exportedLayer->left;
                for (int rowIndex = 0; rowIndex < (int)tile->tileHeight; rowIndex++)
                {
                    uint8_t *destination = composedData + tile->pixelSize * tile->tileWidth * rowIndex * numberOfColumns;
                    destination += tile->pixelSize * currentNormalizedLeft;
                    destination += tile->pixelSize * tile->tileWidth * currentNormalizedTop * numberOfColumns;
                    uint8_t *source = tile->data + tile->pixelSize * tile->tileWidth * rowIndex;
                    size_t size = tile->pixelSize * tile->tileWidth;
                    /* Copy the row of the tile to the composed image */
                    std::memcpy(destination, source, size);
                }
            }
            exportedLayer->data = composedData;
            exportedLayers.push_back(exportedLayer);
        }
    }
    /* Reverse the direction of the vector, so that order is preserved in the Godot mirror universe */
    std::reverse(exportedLayers.begin(), exportedLayers.end());
    return exportedLayers;
}

// ---------------------------------------------------------------------------------------------------------------------
// Clean up memory allocation from the heap.
// ---------------------------------------------------------------------------------------------------------------------
void DestroyKraExportLayers(std::vector<KraExportedLayer *> exportedLayers)
{
    while (exportedLayers.size() > 0)
    {
        KraExportedLayer *layer = exportedLayers.back();
        exportedLayers.pop_back();
        free(layer->data);
        delete layer;
    }
}

KRA_NAMESPACE_END