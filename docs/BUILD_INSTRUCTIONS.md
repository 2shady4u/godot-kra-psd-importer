# build-instructions

Building the plugin can be quite the hassle, since there are several dependencies that have to be 
prepared separately before even being able to compile the plugin. This document aims to serve as a guide
for proper compilation and goes through each of the necessary steps in due detail.

Evidently, this repository needs to be cloned and the submodules need to be initialized using the following Git commands:
```
git clone https://github.com/2shady4u/godot-kra-psd-importer.git
cd godot-kra-psd-importer
git submodule update --init --recursive
```

### Installation pre-requisites:
- Visual Studio Community 2017 or 2019 (Other versions might also work, but weren't tested.)
- SCons
- Git
- CMake (which in most cases includes CMake-GUI)
- ImageMagick, including the development headers (see 'Installation of ImageMagick')
- Clone or download [vcpkg](https://github.com/microsoft/vcpkg) (see 'Building the zipper submodule')

After this, the following steps should be taken:

## 1. Installation of ImageMagick

Godot KRA/PSD Importer requires an installation of ImageMagick on your PC. The required executable can 
be found on the ImageMagick website [here](https://imagemagick.org/script/download.php#windows). For example,
at time of writing, the 'ImageMagick-7.0.10-2-Q16-x64-dll.exe' has to be downloaded and installed. This version
might be updated in the future (whenever ImageMagick release a new version), so it is imperative that the
proper version is checked in the main SConstruct file as found inside of the 'godot-kra-psd-importer'-folder.

During installation it is also required to tick the checkbox which states:  
`- [ ] Install development headers and libraries for C and C++`  
So that the proper headers are included into the installation.

**IMPORTANT**  
In case other versions of ImageMagick than the one initially supported in the plugin are wanted, the relevant DLL,
as found in the 'demo/addons/godot-kra-psd-importer/win64/' also have to replaced with their updated counterparts.
Also, you might have to change the ImageMagick installation path, 'magick_bindings_path', inside of the main 
SConstruct script.

## 2. Compiling the godot-cpp bindings

As is always the case for plugins based on the [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) library, 
the bindings have to be built from scratch using following commands:

```
cd godot-cpp
scons p=windows bits=64 target=release generate_bindings=yes
```

Exporting for the 'release' target is unfortunately a necessity at the moment due to the ImageMagick 
libraries only being readily available in release format.

## 3. Building the zipper submodule

For unzipping the KRA archive, the [zipper](https://github.com/sebastiandev/zipper) submodule is exploited which 
has to be build from scratch as well. One of the pre-requisites of the submodule is the installation of the 'zlib'-library.

### 3a. Installation of zlib

The recommended way to install zlib on Windows is with the help of [vcpkg](https://github.com/microsoft/vcpkg). 
After cloning or downloading this repository, following commands (in PowerShell) are necessary to install the 
correct version of the zlib library and header file:

```
ck vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe install zlib:x64-windows
```

### 3b. Building zipper using CMake-GUI

Now that the zlib-library is installed, the zipper submodule can finally be compiled. For this purpose, the CMake-GUI utility
is used since it automatically detects for any missing libraries and simplifies build issues immensely.

Start up CMake-GUI and set the following variables:  
**Where is the source code:** *<path_to_godot_kra_psd_importer>/zipper*  
**Where to build the binaries:** *<path_to_godot_kra_psd_importer>/zipper/build* (make this folder yourself!!!) 

Click 'configure' and wait for CMake-GUI to ask you for the generator. In that case you have to choose the generator
that agrees to your installed version of Visual Studio Community. 

**IMPORTANT**
CMake-GUI will also ask you about the optional target platform for the generator. It is is imperative that the 
x64 target is chosen, otherwise your compiled Zipper binaries will be incompatible with Godot KRA/PSD Importer.

The program will now nag that it is missing paths to the zlib library and header files. These paths have to be set
by the user to point to the relevant vcpkg files/folders:  
**LIBZ_INCLUDE_DIR** -> *<path_to_vcpkg>/installed/x64-windows/include*  
**LIBZ_LIBRARY** -> *<path_to_vcpkg>/installed/x64-windows/lib/zlib.lib*  

After this, you should be able to press 'configure', 'Generate' and 'Open Project' without any issues. This
will open Visual Studio and here you can build the project in RELEASE mode, hopefully without any errors.
(Some warnings will be spawned, but these can be ignored) Again, the binaries have to be built in release
mode such that they are compatible with the ImageMagick libraries and header files.

Now you'll end up with following DLL's inside of the '/build/Release'-folder:
- Zipper.dll
- zlib1.dll

These files have to be copied and replaced into the 'demo/addons/godot-kra-psd-importer/win64/'-folder
otherwise the plugin won't be able to load the zipper DLL correctly during runtime.

## 4. Building the plugin

After going through all previous steps, the SContruct file found in the repository should be sufficient 
to build this project's C++ source code for Windows, with the help of following command:

```
scons p=windows target=release bits=64
```

This command should be used inside of the 'x64 Native Tools Command Prompt for VS201X', otherwise
SCons won't find the necessary build utilities.

ImageMagick might spawn a lot of warning stating that most of the functionalities require a DLL-file to work.
The necessary DLL-files are included in the /win64-folder so you can safely ignore these warnings without any issue.

And.. that's it! If there are any issues/conflicts when building the plugin that are not answered by this document
then feel free to open an issue or if there are errors in this guide do likewise!
