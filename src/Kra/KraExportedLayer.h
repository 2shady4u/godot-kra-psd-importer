// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once

#include "KraPch.h"

KRA_NAMESPACE_BEGIN

// KraExportedLayer is a structure in which the decompressed binary data for the entire image is saved.
// Needless to say... these structures can become quite big...
struct KraExportedLayer
{
    const wchar_t* name;

    unsigned int channelCount;
    unsigned int x;
    unsigned int y;

    int32_t top;
	int32_t left;
	int32_t bottom;
	int32_t right;

    uint8_t opacity;

    bool isVisible;

    uint8_t* data;
};

KRA_NAMESPACE_END