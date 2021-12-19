# libtf

A library for Text Service Framework

## Build

### Environment

* Visual Studio Community 2019 - 16.11.0
* MSVC v142
* C++ ATL v142
* Windows SDK(10.0.19041.0)

### Setup Project

```sh
cd <path-to-libtf-project>
```

```sh
cmake -S ./ -B ./build/build64-static-rt -DSTATIC_VC_RUNTIME=1 -G "Visual Studio 16 2019" -A x64
cmake -S ./ -B ./build/build32-static-rt -DSTATIC_VC_RUNTIME=1 -G "Visual Studio 16 2019" -A Win32
cmake -S ./ -B ./build/build64 -DSTATIC_VC_RUNTIME=0 -G "Visual Studio 16 2019" -A x64
cmake -S ./ -B ./build/build32 -DSTATIC_VC_RUNTIME=0 -G "Visual Studio 16 2019" -A Win32
```

Build

```sh
cmake --build ./build/build<64|32>[-static-rt] --config Release --target ALL_BUILD
```

## How to use?

This library is a C++ library with a C wrapper

To use it in a C environment, you need to add `libtfdef.h` and `libtf.h` to your include path, and put `libtf.dll` at your program's root path

## What can you do with it?

* Activate/Deactivate Input Method

* Get available InputMethods and KeyboardLayouts in system，which are also called `InputProcessor`s

* Get the profile of the `InputProcessor`, which contains its name and the locale of the language this processor for

* Get and Set active `InputProcessor`

* Get `PreEdit` and `CandidateList` infomation during the `Composition`

## Library Structure

![Library Structure Graph](https://github.com/Windmill-City/libtf/blob/master/docs/IngameIME-TextServiceFramework.png?raw=true)
