#include "../Psd/Psd.h"
#include "../Psd/PsdPlatform.h"
#include "PsdPngExporter.h"

#include <stdio.h>


namespace pngExporter
{
	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
	void SaveMonochrome(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data)
	{
        // Initialise ImageMagick library
        Magick::InitializeMagick(NULL);

        // Create Image object and read in from pixel data above
        Magick::Image image;
        image.read(width, height, "K", MagickCore::CharPixel, data);

        // Write the image to a file.
        image.magick("png"); 
        std::wstring ws(filename);
        std::string str(ws.begin(), ws.end());
        image.write(str);

        // Terminate ImageMagick library
        Magick::TerminateMagick();
	}

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
    void SaveRGB(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data)
    {
        // Initialise ImageMagick library
        Magick::InitializeMagick(NULL);

        uint8_t* colors = new uint8_t[width*height*4u];
        for (unsigned int i=0u; i < height; ++i)
        {
            for (unsigned int j=0u; j < width; ++j)
            {
                const uint8_t r = data[(i*width + j) * 4u + 0u];
                const uint8_t g = data[(i*width + j) * 4u + 1u];
                const uint8_t b = data[(i*width + j) * 4u + 2u];
                const uint8_t a = data[(i*width + j) * 4u + 3u];

                colors[(i*width + j) * 4u + 2u] = r;
                colors[(i*width + j) * 4u + 1u] = g;
                colors[(i*width + j) * 4u + 0u] = b;
                colors[(i*width + j) * 4u + 3u] = a;
            }
        }

        // Create Image object and read in from pixel data above
        Magick::Image image;
        image.read(width, height, "RGB", MagickCore::CharPixel, colors);

        // Write the image to a file.
        image.magick("png"); 
        std::wstring ws(filename);
        std::string str(ws.begin(), ws.end());
        image.write(str);

        // Terminate ImageMagick library
        Magick::TerminateMagick();
    }

	// ---------------------------------------------------------------------------------------------------------------------
	// ---------------------------------------------------------------------------------------------------------------------
    void SaveRGBA(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data)
    {
        // Initialise ImageMagick library
        Magick::InitializeMagick(NULL);

        uint8_t* colors = new uint8_t[width*height*4u];
        for (unsigned int i=0u; i < height; ++i)
        {
            for (unsigned int j=0u; j < width; ++j)
            {
                const uint8_t r = data[(i*width + j) * 4u + 0u];
                const uint8_t g = data[(i*width + j) * 4u + 1u];
                const uint8_t b = data[(i*width + j) * 4u + 2u];
                const uint8_t a = data[(i*width + j) * 4u + 3u];

                colors[(i*width + j) * 4u + 2u] = b;
                colors[(i*width + j) * 4u + 1u] = g;
                colors[(i*width + j) * 4u + 0u] = r;
                colors[(i*width + j) * 4u + 3u] = a;
            }
        }

        // Create Image object and read in from pixel data above
        Magick::Image image;
        image.read(width, height, "RGBA", MagickCore::CharPixel, colors);

        // Write the image to a file.
        image.magick("png"); 
        std::wstring ws(filename);
        std::string str(ws.begin(), ws.end());
        image.write(str);

        // Terminate ImageMagick library
        Magick::TerminateMagick();
    }
}
