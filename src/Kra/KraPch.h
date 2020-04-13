// ############################################################################ #
// Copyright © 2020 Piet Bronders & Jeroen De Geeter <piet.bronders@gmail.com>
// Copyright © 2020 Gamechuck d.o.o. <gamechuckdev@gmail.com>
// Licensed under the MIT License.
// See LICENSE in the project root for license information.
// ############################################################################ #

#pragma once

// This header file contains the bare minimum of includes needed by every translation unit that uses the KRA library.
#include "KraNamespace.h"
#include <stddef.h>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream> 

#define WRITEBUFFERSIZE (8192)

// Enum to define the layer type of the Kra layer.
typedef enum
{
    OTHER,
    PAINT_LAYER,
    VECTOR_LAYER,
    GROUP_LAYER
} kraLayerType;
