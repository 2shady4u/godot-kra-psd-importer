#ifndef GDPSD_H
#define GDPSD_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <ProjectSettings.hpp>
#include <iostream>

#include <Magick++.h>

namespace godot {

class PSD : public Reference {
    GODOT_CLASS(PSD, Reference)

private:
    String psd_file_path;
    String target_folder_path;
    bool verbose_mode;

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