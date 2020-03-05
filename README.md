![Godot PSD Importer banner](icon/godot-psd-importer-banner.png?raw=true "Godot PSD Importer banner")

# godot-psd-importer

**!!!DISCLAIMER !!! WORK IN PROGRESS**

GDNative wrapper for Importing PSD-files into Godot 3.1+, making it possible to significantly speed up development when working with PSD-files. Additionally, it does not require any additional compilation or mucking about with build scripts.

Currently the plugin is only able to export to the Truevision TGA format. Support for the PNG format is part of the roadmap and will be implemented asap.

### Supported operating systems:
- Windows

This is an unfortunate constraint of the [PSD Library SDK](https://molecular-matters.com/products_psd_sdk.html), a library that is used to make this plugin work. Efforts to make this library accessible in Linux are ongoing.

# Roadmap

- [ ] Export to PNG
- [ ] Support for Krita *.kra*-files
- [ ] Creation of a PackedScene that mirrors the PSD/KRA layer structure.
- [ ] Addition of metadata interface to create advanced functionalities. (parallax, tweening, ...)

# How to install?

Re-building Godot from scratch is **NOT** required, the proper way of installing this plugin is to either install it through the Asset Library or to just manually download the build files yourself.

### Godot Asset Library

**Godot-PSD-Importer** is available through the official Godot Asset Library, and can be installed in the following way:

- Click on the 'AssetLib' button at the top of the editor.
- Search for 'godot-psd-importer' and click on the resulting element.
- In the dialog pop-up, click 'Download'.
- Once the download is complete, click on the install button...
- Once more, click on the 'Install' button.
- Activate the plugin in the 'Project Settings/Plugins'-menu.
- All done!

### Manually

It's also possible to manually download the build files found in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab, extract them on your system and run the supplied demo-project. Make sure that Godot is correctly loading the *gdsqlite.gdns*-resource and that it is available in the *res://*-environment.

An example project, named "demo", can also be downloaded from the releases tab. 

# Usage

After succesful activation of the plugin, an additional panel should have appeared next to the 'Import'-panel.

![Import Plugin Editor](readme/import_plugin_editor.PNG?raw=true "Import Plugin Editor")

Several fields are available:
- **PSD File** (String ,default value='res://addons/godot-psd-importer/examples/Sample.psd')

Path to the **.psd*-file that is to be exported.

- **Target Folder** (String, default='res://graphics/')

Folder to which the exported layers of the **.psd*-file, as given by 'PSD File', will be saved.

After setting these fields correctly, the 'import'-button can be used to start the importing process.

# Credits

This plugin makes heavy use of the [PSD Library SDK](https://molecular-matters.com/products_psd_sdk.html), without which this plugin would be impossible.

# Advanced instructions

For more advanced usage, the binaries can also be called directly from GDScript without using the editor plugin at all.
This might be preferred in some more obscure/creative cases and is fully supported by the plugin.

## Variables

- **psd_file_path** (String ,default value='res://addons/godot-psd-importer/examples/Sample.psd')

Path to the *.psd-file that is to be exported. Both *res://* and *user://* keywords can be used to define the path.

- **target_folder_path** (String, default='res://graphics/')

Folder to which the exported layers of the *.psd-file, as given by psd_file_path, will be saved. Both *res://* and *user://* keywords can be used to define the folder.

- **verbose_mode** (Boolean, default=false)

Setting verbose_mode on True results in an information dump in the Godot console that is handy for .

## Functions

- Boolean success = **import_psd()**



