#ifndef GDKRAPSDIMPORTER_H
#define GDKRAPSDIMPORTER_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Directory.hpp>
#include <ProjectSettings.hpp>
#include <Magick++.h>

#include <vector>
#include <fstream>
#include <ostream>
#include <string>
#include <map>

namespace godot {

class KRAPSDImporter : public Reference {
    GODOT_CLASS(KRAPSDImporter, Reference)

typedef enum
{
    PNG,
    TGA
} EXPORT_TYPE;

typedef enum
{
    KRA,
    PSD
} IMPORT_TYPE;

typedef enum
{
    MONOCHROME,
    RGB,
    RGBA
} CHANNEL_TYPE;

private:
    String rawFilePath;
    String targetFolderPath;
    String errorMessage;

    bool verboseMode;
    bool cropToCanvas;

    int exportType;
    int importType;
    int channelType;

    float resizeFactor;

    bool exportAllPSDLayers();
    bool exportAllKRALayers();
    bool SaveTexture(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data);

public:
    static void _register_methods();

    KRAPSDImporter();
    ~KRAPSDImporter();

    void _init();

    bool exportAllLayers();
    int test();
};

}

#endif