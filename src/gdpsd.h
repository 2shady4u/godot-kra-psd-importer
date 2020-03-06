#ifndef GDPSD_H
#define GDPSD_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <ProjectSettings.hpp>
#include <iostream>

namespace godot {

class PSD : public Reference {
    GODOT_CLASS(PSD, Reference)

typedef enum
{
  PNG,
  TGA
} ExportType;

private:
    String psd_file_path;
    String target_folder_path;
    bool verbose_mode;
    int export_type;

public:
    static void _register_methods();

    PSD();
    ~PSD();

    void _init();

    int export_psd();
    int test();
};

}

#endif