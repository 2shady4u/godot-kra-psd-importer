// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once

#include "KraPch.h"
#include "KraTile.h"

KRA_NAMESPACE_BEGIN

// KraLayer is a structure in which general properties for a KRA layer are stored.
// The actual image data is found in the tiles vector.
struct KraLayer
{
    const wchar_t* name;
    const char* filename;

    unsigned int channelCount;
    unsigned int x;
    unsigned int y;

    uint8_t opacity;

    uint32_t type;
    bool isVisible;

    std::vector<std::unique_ptr<KraTile>> tiles;

    bool corruptionFlag = false;
};

KRA_NAMESPACE_END