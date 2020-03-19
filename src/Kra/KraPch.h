#pragma once


// This header file contains the bare minimum of includes needed by every translation unit that uses the KRA library.
#include "KraNameSpace.h"
#include <stddef.h>
#include <string>
#include <vector>

typedef enum
{
    MONOCHROME,
    RGB,
    RGBA
} COLOR_SPACE;

typedef enum
{
    PAINT_LAYER,
    VECTOR_LAYER
} LAYER_TYPE;