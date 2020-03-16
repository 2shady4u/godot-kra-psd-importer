#ifndef GDPSDIMPORTER_H
#define GDPSDIMPORTER_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Directory.hpp>
#include <ProjectSettings.hpp>
#include <Magick++.h>
#include <zipper.h>
#include <unzipper.h>
#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <fstream>
#include <ostream>
#include <string>
#include <map>

namespace godot {

class PSDImporter : public Reference {
    GODOT_CLASS(PSDImporter, Reference)

typedef enum
{
    PNG,
    TGA
} EXPORT_TYPE;

typedef enum
{
    MONOCHROME,
    RGB,
    RGBA
} CHANNEL_TYPE;

private:
    String psdFilePath;
    String targetFolderPath;
    String errorMessage;

    bool verboseMode;
    bool cropToCanvas;

    int exportType;
    int channelType;

    float resizeFactor;

    static int lzff_decompress(const void* input, int length, void* output, int maxout);

public:
    static void _register_methods();

    PSDImporter();
    ~PSDImporter();

    void _init();

    bool exportAllLayers();
    bool SaveTexture(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data);
    int test();
};

}

#endif