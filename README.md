![Godot KRA/PSD Importer banner](icon/godot-kra-psd-importer-banner.png?raw=true "Godot KRA/PSD Importer banner")

# godot-kra-psd-importer

**!!!DISCLAIMER !!! WORK IN PROGRESS**

GDNative wrapper for Importing KRA- and PSD-files into Godot 3.1+, making it possible to significantly speed up development. Additionally, it does not require any additional compilation or mucking about with build scripts.

Currently the plugin is able to export to both PNG and TGA image formats.

### Supported operating systems:
- Windows

This is an unfortunate constraint of the [PSD Library SDK](https://molecular-matters.com/products_psd_sdk.html), a library that is used to make this plugin work. Efforts to make this library accessible in Linux are ongoing.

The KRA-component of this plugin might be released separately in the future for Linux and/or MacOS.

### Table Of Contents

- [Roadmap](#roadmap)
- [Installation](#installation)
- [Usage](#usage)
- [Credits](#credits)
- [Blatant self-advertisement](#blatant-self-advertisement)
- [Advanced usage instructions](#advanced-usage-instructions)
- [Build instructions](#build-instructions)
- [Additional documentation](#additional-documentation)

# <a name="roadmap">Roadmap</a>

- [x] Export to PNG
- [x] Support for Krita *.kra*-files
- [ ] Creation of a PackedScene that mirrors the PSD/KRA layer structure.
- [ ] Addition of metadata interface to create advanced functionalities. (parallax, tweening, ...)

# <a name="installation">installation</a>

Re-building Godot from scratch is **NOT** required, the proper way of installing this plugin is to either install it through the Asset Library or to just manually download the build files yourself.

Additionally, either setting the environment variable **MAGICK_CODER_MODULE_PATH** in the system registry (recommended way) or an installation of ImageMagick is required to be able to export to the PNG- and TGA-formats.  
The variable should be pointed to the following path:

`MAGICK_CODER_MODULE_PATH = <path-to-godot-project>\addons\godot-kra-psd-importer\bin\win64`

### Godot Asset Library

The **Godot KRA/PSD Importer** is available through the official Godot Asset Library, and can be installed in the following way:

- Click on the 'AssetLib' button at the top of the editor.
- Search for 'godot-kra-psd-importer' and click on the resulting element.
- In the dialog pop-up, click 'Download'.
- Once the download is complete, click on the install button...
- Once more, click on the 'Install' button.
- Activate the plugin in the 'Project Settings/Plugins'-menu.
- All done!

### Manually

It's also possible to manually download the build files found in the [releases](https://github.com/2shady4u/godot-kra-psd-importer/releases) tab, extract them on your system and run the supplied demo-project. Make sure that Godot is correctly loading the *gdsqlite.gdns*-resource and that it is available in the *res://*-environment.

An example project, named "demo", can also be downloaded from the releases tab. 

# <a name="usage">Usage</a>

After succesful activation of the plugin, an additional panel should have appeared next to the 'Import'-panel.

![Import Plugin Editor](readme/import_plugin_editor.PNG?raw=true "Import Plugin Editor")

Several fields are available:
- **Raw File** (String ,default value='res://addons/godot-psd-importer/examples/PSDExample.psd')

Path to the **.kra*- or **.psd*-file that is to be imported into the engine.

- **Target Folder** (String, default='res://')

Folder to which the exported layers of the **.kra*- or **.psd*-file, as given by 'Raw File', will be saved.

- **Export Type** (Int, default=ExportType.PNG)

Image format to which the exported layers of the raw file will be converted.

- **Crop Layers to Canvas** (Bool, default=true)

Crop all layers of the raw file to the size of the canvas, making them the exact same size and removing any content that spills out of the canvas. Alternatively, the canvas size is ignored and the layer sizes are respected, resulting in exported images with possibly different sizes. 

**IMPORTANT**: This option is currently unavailable for **.kra*-files and thus their image canvas size will always be ignored.

- **Mirror Universe** (Bool, default=true)

Creates a mirrored version of the raw file's layer structure. Current implementation is extremely elementary without any respect for groups or other more advanced functionalities.

- **Resize Factor** (Float, default=1.0)

Rescales the exported textures with a certain factor.

After setting these fields correctly, the 'import'-button can be used to start the importing process.  
Be sure to check console if any errors are spawned that cannot be decyphered by the import dock.

# <a name="credits">Credits</a>

This plugin makes heavy use of the [PSD Library SDK](https://molecular-matters.com/products_psd_sdk.html), without which this plugin would be impossible. Alongside this, both the [Zipper](https://github.com/sebastiandev/zipper) and [TinyXML-2](https://github.com/leethomason/tinyxml2) projects are exploited to significantly simplify the import process of *.kra*-files.

For enabling PNG and TGA exporting functionalities, the [ImageMagick](https://imagemagick.org/index.php) libraries are employed and the necessary *.dll are included in the addon. 

# <a name="blatant-self-advertisement">Blatant self-advertisement</a>

This plugin was made by Gamechuck and is actively used during development of [Trip the Ark Fantastic](https://www.tripthearkfantastic.com/), a colourful game about societal and scientific themes set in the fabled Animal Kingdom. Be sure to check it out!

[![Trip the Ark Fantastic Banner](readme/ark_fantastic_presskit_header.png?raw=true "Trip the Ark Fantastic Banner")](https://www.tripthearkfantastic.com/)

# <a name="advanced-usage-instructions">Advanced usage instructions</a>

For more advanced usage, the binaries can also be called directly from GDScript without using the editor plugin at all.
This might be preferred in some more obscure/creative cases and is fully supported by the plugin.

## Variables

- **raw_file_path** (String ,default value='res://addons/godot-psd-importer/examples/PSDExample.psd')

Path to the *.kra- or *.psd-file that is to be exported. Both *res://* and *user://* keywords can be used to define the path.

- **target_folder_path** (String, default='res://')

Folder to which the exported layers of the raw file, as given by raw_file_path, will be saved. Both *res://* and *user://* keywords can be used to define the folder.

- **export_type** (int, default=0)

    * PNG (= 0)
    * TGA (= 1)

Image format to which the layers of raw file will be exported. Both PNG and TGA formats are available.

- **crop_to_canvas** (Boolean, default=true)

Either crop the layers to the canvas or ignore the canvas size entirely and respect the layer sizes.

- **mirror_universe** (Boolean, default=true)

If set to true, the importer will emit the "texture_created"-signal whenever a new texture is created. Check the implementation of the 'create_packed_scene.gd'-script for more information.

- **resize_factor** (Float, default=1.0)

Factor with which the exported textures will be resized.

- **verbose_mode** (Boolean, default=false)

Setting verbose_mode on true results in an information dump in the Godot console that is handy for debugging purposes.

## Functions

- Boolean success = **export_all_layers()**

Exports the layers of a **.kra*- or **.psd*-file, as given by raw_file_path, to a target folder, as given by target_folder_path. Exported image format is either PNG or TGA as given by the export_type variable.

# <a name="build-instructions">Build instructions</a>

Instructions for building the **Godot KRA/PSD Importer** from scratch are available [here](docs/BUILD_INSTRUCTIONS.md).

# <a name="additional-documentation">Additional documentation</a>

The KRA-format is not that well-documented and thus this library attempts to reverse-engineer the format from scratch.
A small document discussing the KRA-format and its pecularities is available [here](docs/KRA_FORMAT.md).
