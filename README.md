# EasyDialogEditor

**EasyDialogEditor** is a lightweight dialog tree editor for games. 

It allows you to visually create dialog trees and export them as JSON.
It is not a dialog system, but rather a way of visually creating dialog data. As such, it will work on any dialog system as long as you parse the data on your end.

It is a single portable executable with no external dependencies required. 

Built with `ImGui`, `ImNodes`, and `SDL3`.

It currently only works on **Windows**.

<img src="https://github.com/user-attachments/assets/b8f0e885-0daa-4acf-a63a-2e986947150d" width="477" height="257"/>

## How to build

### Prerequisites
- C++ Compiler (GCC, Clang, or MSVC)
- CMake (Minimum version required, e.g., 3.20+)

### Building
Inside the cloned project run the following command:

```
git submodule update --init --recursive
```

Some dependancies are managed by [vcpkg](https://github.com/microsoft/vcpkg), therefore we need to bootstrap **vcpkg** and install the dependancies:
```
cd vcpkg
.\bootstrap-vcpkg.bat
vcpkg install sdl2 nlohmann-json
```
Alternatively, instead of running `vcpkg install sdl2 nlohmann-json`, if you are using Visual Studio 2022:
- Right click the **EasyDialogEditor** solution → Properties → vcpkg → Set `Use Vcpkg Manifest` to **Yes**

## FAQ (Frequently Asked Questions)

#### Q: Why does windows suspect a virus when opening EasyDialogManager.exe from Releases?
R: Windows will almost always give you that warning when running a downloaded executable. There is not much I can do, but you can always inspeact the code and build from source.

#### Q: Does it work on Linux/MacOs ?
R: It currently only runs natively on Windows, but you may be able to run it on Linux using *Wine* (I didn't test it though).
