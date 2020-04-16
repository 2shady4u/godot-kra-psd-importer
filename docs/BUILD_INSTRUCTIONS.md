# build-instructions

This document aims to serve as a guide for proper compilation and goes through each of the necessary steps in due detail.

Evidently, this repository needs to be cloned and the submodules need to be initialized using the following Git commands:
```
git clone https://github.com/2shady4u/godot-kra-psd-importer.git
cd godot-kra-psd-importer
git submodule update --init --recursive
```

The remainder of this document assumes that this plugin is being compiled for Windows hosts. Compilation for Linux and MacOS hosts is largely similar.

### Installation pre-requisites:
- Visual Studio Community 2017 or 2019 (Other versions might also work, but weren't tested.)
- SCons
- Git

After this, the following steps should be taken:

## 1. Copy-pasting and renaming '*pnglibconf.h.prebuilt*'

Since these build instructions do not use any CMake utilities, the auto-generated config files for libpng are not.. well.. auto-generated. Thus, to circumvent this, the makers of libpng were kind enough to supply a default configuration for their library.
This default configuration file can be found at:

`<path-to-repository>/godot-kra-psd-importer/libpng/scripts/gpnglibconf.h.prebuilt`

Take this file, copy-paste it inside of the main libpng folder and rename it as such:

`<path-to-repository>/godot-kra-psd-importer/libpng/gpnglibconf.h`

## 2. Compiling the godot-cpp bindings

As is always the case for plugins based on the [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) library, 
the bindings have to be built from scratch using following commands:

```
cd godot-cpp
scons p=windows bits=64 target=release generate_bindings=yes
```

Both debug and release targets are supported so feel free choose whichever is more suitable for your purposes.

## 3. Building the plugin

After going through all previous steps, the SContruct file found in the repository should be sufficient 
to build this project's C++ source code for Windows, with the help of following command:

```
scons p=windows target=release bits=64
```

This command should be used inside of the 'x64 Native Tools Command Prompt for VS201X', otherwise
SCons won't find the necessary build utilities.

For linux and/or MacOS builds the platform variable has to be replaced by either `p=X11` or `p=osx` respectively.

And.. that's it! If there are any issues/conflicts when building the plugin that are not answered by this document
then feel free to open an issue or if there are errors in this guide do likewise!
