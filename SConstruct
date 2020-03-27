#!/usr/bin/env python
import os, subprocess

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

# Due to the nature of its dependencies, this library/plugin only supports release targets.
# Define our options
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(EnumVariable('p', "Compilation target, alias for 'platform'", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', 'demo/addons/godot-kra-psd-importer/bin/'))
opts.Add(PathVariable('target_name', 'The library name.', 'lidgdkrapsdimporter', PathVariable.PathAccept))
opts.Add(PathVariable('vcpkg_path', 'The path to the installed vcpkg libraries.', 'C:/Users/pietb/Documents/Repositories/External/vcpkg/installed/'))

# Local dependency paths, adapt them to your setup
godot_headers_path = "godot-cpp/godot_headers/"
cpp_bindings_path = "godot-cpp/"
zipper_bindings_path = "zipper/"
cpp_library = "libgodot-cpp"

# Currently only supports 64-bit architectures, 32-bit architectures might be possible.
bits = 64

# Updates the environment with the option variables.
opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

if env['p'] != '':
    env['platform'] = env['p']

if env['platform'] == '':
    print("No valid target platform selected.")
    quit()

# Check our platform specifics
if env['platform'] == "osx":
    env['target_path'] += 'osx/'
    cpp_library += '.osx'
    env.Append(CCFLAGS = ['-g','-O3', '-arch', 'x86_64'])
    env.Append(CXXFLAGS = ['-std=c++17']) 
    env.Append(LINKFLAGS = ['-arch', 'x86_64'])

    env['vcpkg_path'] += '/x64-osx/'

elif env['platform'] in ('x11', 'linux'):
    env['target_path'] += 'x11/'
    cpp_library += '.linux'
    env.Append(CCFLAGS = ['-fPIC','-g','-O3'])
    env.Append(CXXFLAGS = ['-std=c++17']) 

    env['vcpkg_path'] += '/x64-linux/'

elif env['platform'] == "windows":
    env['target_path'] += 'win64/'
    cpp_library += '.windows'
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV = os.environ)
    env.Append(CCFLAGS = ['-DWIN32', '-D_WIN32', '-D_WINDOWS', '-W3', '-GR', '-D_CRT_SECURE_NO_WARNINGS'])
    env.Append(CCFLAGS = ['-O2', '-EHsc', '-DNDEBUG', '-MD'])

    env['vcpkg_path'] += '/x64-windows/'

# Create the path to the godot-cpp bindings library.
cpp_library += '.release'
cpp_library += '.' + str(bits)

# make sure our binding library is properly included
env.Append(CPPPATH=['.', 
godot_headers_path,
cpp_bindings_path + 'include/', 
cpp_bindings_path + 'include/core/', 
cpp_bindings_path + 'include/gen/',
env['vcpkg_path'] + 'include/',
env['vcpkg_path'] + 'include/Magick++/',
env['vcpkg_path'] + 'include/wand/',
env['vcpkg_path'] + 'include/magick/',
env['vcpkg_path'] + 'include/lzma/',
env['vcpkg_path'] + 'include/libpng16/',
env['vcpkg_path'] + 'include/freetype/',
zipper_bindings_path + 'zipper/'])
#zipper_bindings_path + 'zipper/tps/'])
env.Append(LIBPATH=[cpp_bindings_path + 'bin/', env['vcpkg_path'] + 'lib/', zipper_bindings_path + 'build/Release/'])
env.Append(LIBS=[cpp_library, 
"bz2", 
"freetype",
"graphicsmagick",
"jpeg",
"libpng16",
"lzma",
"tiff",
"tiffxx",
"turbojpeg",
"Zipper"])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/'])
sources = [Glob('src/*.cpp'), Glob('src/Psd/*.cpp'), Glob('src/Kra/*.cpp'), 'src/tinyxml2/tinyxml2.cpp', 'src/Psd/Psdminiz.c']

library = env.SharedLibrary(target=env['target_path'] + env['target_name'] , source=sources)

Default(library)

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
