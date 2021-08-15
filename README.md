# libtf

A library for Text Service Framework

## Build

### Environment

- Visual Studio Community 2019 - 16.11.0
- MSVC v142
- C++ ATL v142
- Windows SDK(10.0.19041.0)

Run Command

```bash
cmake --build ./build --config Release --target ALL_BUILD
```

## How to use?

This is a library that can be used in a `Pure C Environment`

Only need to include two header files, `libtf.h` and `libtfdef.h`

## What can you do with it?

- Control open status of the Input Method

- Obtain System available Input Methods as well as Keyboard Layouts，which also called `Input Processor`

- Obtain the localized name of the `Input Processor` as well as the display name of the `locale` it associate to

- Obtain or set the active `Input Processor`

- Watch the change of `Conversion` / `Sentence` Mode

- Obtain the `PreEdit` text and the `Candidate` list of the `Composition`

- Hide the Candidate Window of the Input Method and draw it yourself
