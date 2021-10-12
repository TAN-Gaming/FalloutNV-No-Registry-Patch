# FalloutNV-No-Registry-Patch
No-Registry Patch for Fallout New Vegas (GOG Version)

[release_link]: https://github.com/TAN-Gaming/FalloutNV-No-Registry-Patch/releases

## Features
- It fixes launching problems after moving the game to another folder.
- It doesn't touch the Registry at all.
- It makes the game portable.

## How to use
- Download release builds from the [release section][release_link] or build from source.
- Find the file `GalaxyWrp.dll` in the game folder, rename it to `orig_GalaxyWrp.dll`.
- Paste the downloaded file into the game folder.

## Build instructions

### Requirements
- MinGW-W64 compiler (version >= 8.0)
- CMake (version >= 3.15)

### Building the DLL
```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE="Release" -DBUILD_MINGW_STATIC=ON -S"your_source_dir" -B"your_build_dir"
cd your_build_dir
mingw32-make -f Makefile
```

## Notes
- MSVC is not supported because it's a lot easier to create a proxy DLL using MinGW.
- This code may works with another version if you can create a proxy DLL that loaded by `FalloutNV.exe` and `FalloutNVLauncher.exe`.
