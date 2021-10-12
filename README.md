# FalloutNV-No-Registry-Patch
No-Registry Patch for Fallout New Vegas (GOG Version)


[release_link]: https://github.com/TAN-Gaming/FalloutNV-No-Registry-Patch/releases

# Features
- Fix install-related problems.
- Make your game portable.

# How to use
- Download release builds from the [release section][release_link] or build from source.
- Find the file `GalaxyWrp.dll` in your game folder, rename it to `orig_GalaxyWrp.dll`.
- Paste the downloaded file into your game folder.

# Build instructions

## Requirements
- MinGW-W64 compiler (version >= 8.0)
- CMake (version >= 3.15)

## Building the DLL
```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE="Release" -DBUILD_MINGW_STATIC=ON -S"your_source_code_dir" -B"your_build_dir"
cd your_build_dir
mingw32-make -f Makefile
```
