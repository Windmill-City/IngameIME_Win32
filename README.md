# libtf

A library for Text Service Framework

## Build

### Environment

- Visual Studio Community 2019 - 16.11.0
- MSVC v142
- C++ ATL v142
- Windows SDK(10.0.19041.0)

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

This is a library that can be used in a `Pure C Environment`

Only need to include two header files, `libtf.h` and `libtfdef.h`

## What can you do with it?

- Control open status of the Input Method

- Obtain System available Input Methods as well as Keyboard Layouts，which also called `Input Processor`

- Obtain the localized name of the `Input Processor` as well as the display name of the `locale` it associate to

- Obtain or set the active `Input Processor`

- Listen for status changes of `Conversion Mode` and `Sentence Mode`

- Obtain the `PreEdit` text and the `Candidate` list of the `Composition`

- Hide the Candidate Window of the Input Method and draw it yourself
