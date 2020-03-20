// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once

#include "KraPch.h"
#include "KraLayer.h"

KRA_NAMESPACE_BEGIN

// KraTile is a structure in which the general properties of a KRA document/archive are stored.
// Each KRA archive consists of one or more layers (stored in a vector) that contain actual data.
struct KraDocument
{
	unsigned int width;
	unsigned int height;
	unsigned int channelCount;

    const char* name;

    std::vector<KraLayer*> layers;

	bool corruptionFlag = false;
};

KRA_NAMESPACE_END