// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once

#include "KraPch.h"

KRA_NAMESPACE_BEGIN

// KraTile is a structure in which the decompressed binary data for a single tile is stored.
// Each KRA layer consists of a vector of tiles that hold the actual image data.
struct KraTile
{

    // Version statement of the tile, always equal to 2.
    unsigned int version;
    // Number of vertical pixels stored in the tile, always equal to 64.
    unsigned int tileHeight;
    // Number of horizontal pixels stored in the tile, always equal to 64.
    unsigned int tileWidth;
    // Number of elements in each pixel, is equal to 4 for RGBA.
    unsigned int pixelSize;

    // These can also be negative!!!
    // The left (X) position of the tile.
    int32_t left;
    // The top (Y) position of the tile.
    int32_t top;

    // Number of compressed bytes that represent the tile data.
    int compressedLength;
    // Number of decompressed bytes that represent the tile data.
    // Always equal to tileHeight * tileWidth * pixelSize
    int decompressedLength;

    // Decompressed image data of this tile.
    uint8_t* data;

    // Flag that gets raised when something goes wrong when parsing the data.
    bool corruptionFlag = false;
};

KRA_NAMESPACE_END