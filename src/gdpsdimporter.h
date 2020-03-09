#ifndef GDPSDIMPORTER_H
#define GDPSDIMPORTER_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Directory.hpp>
#include <ProjectSettings.hpp>
#include <iostream>

namespace godot {

class PSDImporter : public Reference {
    GODOT_CLASS(PSDImporter, Reference)

typedef enum
{
  PNG,
  TGA
} ExportType;

private:
    String psd_file_path;
    String target_folder_path;
    String error_message;
    bool verbose_mode;
    bool crop_to_canvas;
    int export_type;

public:
    static void _register_methods();

    PSDImporter();
    ~PSDImporter();

    void _init();

    bool export_all_layers();
    int test();
};

}

#endif