#!/usr/bin/env python

import os
import sys
import subprocess

#################
#OPTIONS#########
#################

# Try to detect the host platform automatically.
# This is used if no `platform` argument is passed
if sys.platform.startswith('linux'):
    host_platform = 'linux'
elif sys.platform == 'darwin':
    host_platform = 'osx'
elif sys.platform == 'win32' or sys.platform == 'msys':
    host_platform = 'windows'
else:
    raise ValueError(
        'Could not detect platform automatically, please specify with '
        'platform=<platform>'
    )

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

# Define our options
opts.Add(EnumVariable(
    'platform',
    'Target platform',
    host_platform,
    allowed_values=('linux', 'osx', 'windows', 'ios'),
    ignorecase=2
))
opts.Add(EnumVariable(
    'bits',
    'Target platform bits',
    'default',
    ('default', '32', '64')
))
opts.Add(BoolVariable(
    'use_llvm',
    'Use the LLVM compiler - only effective when targeting Linux',
    False
))
opts.Add(BoolVariable(
    'use_mingw',
    'Use the MinGW compiler instead of MSVC - only effective on Windows',
    False
))
# Must be the same setting as used for cpp_bindings
opts.Add(EnumVariable(
    'target',
    'Compilation target',
    'debug',
    allowed_values=('debug', 'release'),
    ignorecase=2
))
opts.Add(PathVariable(
    'target_path', 
    'The path where the lib is installed.', 
    'demo/bin/'
))
opts.Add(PathVariable(
    'target_name', 
    'The library name.', 
    'libgdexample', 
    PathVariable.PathAccept
))

# Local dependency paths, adapt them to your setup
godot_headers_path = "godot-cpp/godot_headers/"
cpp_bindings_path = "godot-cpp/"

env = Environment(ENV = os.environ)
# Updates the environment with the option variables.
opts.Update(env)
# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))

is64 = sys.maxsize > 2**32
if (
    env['TARGET_ARCH'] == 'amd64' or
    env['TARGET_ARCH'] == 'emt64' or
    env['TARGET_ARCH'] == 'x86_64' or
    env['TARGET_ARCH'] == 'arm64-v8a'
):
    is64 = True

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# This makes sure to keep the session environment variables on Windows.
# This way, you can run SCons in a Visual Studio 2017 prompt and it will find
# all the required tools
if host_platform == 'windows':
    if env['bits'] == '64':
        env = Environment(TARGET_ARCH='amd64')
    elif env['bits'] == '32':
         env = Environment(TARGET_ARCH='x86')
    opts.Update(env)

if env['bits'] == 'default':
    env['bits'] = '64' if is64 else '32'

arch_suffix = env['bits']

###################
####FLAGS##########
###################

if env['platform'] == 'linux':

    if env['use_llvm']:
        env['CC'] = 'clang'
        env['CXX'] = 'clang++'

    env.Append(CCFLAGS=['-fPIC'])
    env.Append(CXXFLAGS=['-std=c++17'])
    if env['target'] == 'debug':
        env.Append(CCFLAGS = ['-g3','-Og'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS = ['-g','-O3'])

elif env['platform'] == 'osx':

    # Use Clang on macOS by default
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

    if env['bits'] == '32':
        raise ValueError(
            'Only 64-bit builds are supported for the macOS target.'
        )

    # For compiling zlib on macOS, an additional compiler flag needs to be added!
    # See: https://github.com/HaxeFoundation/hxcpp/issues/723
    env.Append(CCFLAGS=['-g', '-arch', 'x86_64', '-DHAVE_UNISTD_H'])
    env.Append(CXXFLAGS=['-std=c++17'])
    env.Append(LINKFLAGS=[
        '-arch',
        'x86_64',
        '-framework',
        'Cocoa',
        '-Wl,-undefined,dynamic_lookup',
    ])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-O2'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])

elif env['platform'] == "windows":
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV = os.environ)
    env.Append(CCFLAGS = ['-DWIN32', '-D_WIN32', '-D_WINDOWS', '-W3', '-GR', '-D_CRT_SECURE_NO_WARNINGS'])
    if env['target'] == 'debug':
        env.Append(CCFLAGS = ['-EHsc', '-D_DEBUG', '-MDd'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS = ['-O2', '-EHsc', '-DNDEBUG', '-MD'])

#####################
#ADD SOURCES#########
#####################
cpp_bindings_libname = 'libgodot-cpp.{}.{}.{}'.format(
                        env['platform'],
                        env['target'],
                        arch_suffix)

# make sure our binding library is properly included
env.Append(CPPPATH=['.', 
godot_headers_path,
cpp_bindings_path + 'include/', 
cpp_bindings_path + 'include/core/', 
cpp_bindings_path + 'include/gen/',
'zlib/'])
env.Append(LIBS=[cpp_bindings_libname])
env.Append(LIBPATH=[cpp_bindings_path + 'bin/'])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/'])
sources = [
Glob('src/*.cpp'), 
Glob('src/Kra/*.cpp'),
'src/tinyxml2/tinyxml2.cpp',
Glob('libpng/*.c',exclude=['libpng/pngtest.c']),
Glob('zlib/*.c'),
Glob('zlib/contrib/minizip/unzip.c'),
Glob('zlib/contrib/minizip/ioapi.c')
]

# Add sources for including PSD-support! Only on Windows!
if env['platform'] == "windows":
    sources += [Glob('src/Psd/*.cpp'), 'src/Psd/Psdminiz.c']

###############
#BUILD LIB#####
###############

library = env.SharedLibrary(target=env['target_path'] + env['target_name'] , source=sources)
