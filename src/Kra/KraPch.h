#pragma once


// This header file contains the bare minimum of includes needed by every translation unit that uses the KRA library.
#include "KraNameSpace.h"
#include <stddef.h>

typedef enum
{
    MONOCHROME,
    RGB,
    RGBA
} COLOR_SPACE;