#ifndef GDKRAPSDIMPORTER_H
#define GDKRAPSDIMPORTER_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Directory.hpp>
#include <ProjectSettings.hpp>
#include <libpng/png.h>
#include "avir/avir.h"

#include <vector>
#include <fstream>
#include <ostream>
#include <string>
#include <map>

#ifdef WIN32
// the main include that always needs to be included in every translation unit that uses the PSD library
#include "Psd/Psd.h"

// for convenience reasons, we directly include the platform header from the PSD library.
// we could have just included <Windows.h> as well, but that is unnecessarily big, and triggers lots of warnings.
#include "Psd/PsdPlatform.h"

#include "Psd/PsdMallocAllocator.h"
#include "Psd/PsdNativeFile.h"

#include "Psd/PsdDocument.h"
#include "Psd/PsdColorMode.h"
#include "Psd/PsdLayer.h"
#include "Psd/PsdChannel.h"
#include "Psd/PsdChannelType.h"
#include "Psd/PsdLayerMask.h"
#include "Psd/PsdVectorMask.h"
#include "Psd/PsdLayerMaskSection.h"
#include "Psd/PsdImageDataSection.h"
#include "Psd/PsdImageResourcesSection.h"
#include "Psd/PsdParseDocument.h"
#include "Psd/PsdParseLayerMaskSection.h"
#include "Psd/PsdParseImageDataSection.h"
#include "Psd/PsdParseImageResourcesSection.h"
#include "Psd/PsdLayerCanvasCopy.h"
#include "Psd/PsdInterleave.h"
#include "Psd/PsdPlanarImage.h"
#include "Psd/PsdExport.h"
#include "Psd/PsdExportDocument.h"
#include "Psd/PsdLayerType.h"

PSD_PUSH_WARNING_LEVEL(0)
// disable annoying warning caused by xlocale(337): warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable : 4530)
#include <string>
#include <sstream>
PSD_POP_WARNING_LEVEL
#endif

#include "Kra/KraDocument.h"
#include "Kra/KraExportedLayer.h"
#include "Kra/KraParseDocument.h"
#include "Kra/KraExport.h"

namespace godot {

class KRAPSDImporter : public Reference {
    GODOT_CLASS(KRAPSDImporter, Reference)

//typedef enum
//{
//    PNG,
//    TGA
//} EXPORT_TYPE;

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
} COLOR_SPACE_NAME;

private:
    String rawFilePath;
    String targetFolderPath;
    String errorMessage;

    bool verboseMode;
    bool cropToCanvas;
    bool mirrorUniverse;

    //int exportType;
    int importType;
    int channelType;

    float resizeFactor;

    #ifdef WIN32
    bool ExportAllPSDLayers();
    bool EmitPSDTextureProperties(std::wstring filename, psd::Layer* layer);
    #endif
    bool ExportAllKRALayers();
    bool EmitKRATextureProperties(std::wstring filename, const std::unique_ptr<kra::KraExportedLayer> &layer);
    std::wstring ExportLayer(const wchar_t* name, unsigned int width, unsigned int height, const uint8_t* data);
    bool SaveTexture(const wchar_t* filename, unsigned int width, unsigned int height, const uint8_t* data);

public:
    static void _register_methods();

    KRAPSDImporter();
    ~KRAPSDImporter();

    void _init();

    bool ExportAllLayers();
};

}

#endif
