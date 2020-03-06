#pragma once

#include "../Psd/Psdstdint.h"
#include <Magick++.h>

namespace pngExporter
{
	/// Assumes 8-bit single-channel data.
	void SaveMonochrome(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data);

	/// Assumes 8-bit RGBA data, but ignores alpha (32-bit data is assumed for performance reasons).
	void SaveRGB(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data);

	/// Assumes 8-bit RGBA data.
	void SaveRGBA(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data);
}
