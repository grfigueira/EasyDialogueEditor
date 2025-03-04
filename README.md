# EasyDialogEditor

**EasyDialogEditor** is a lightweight dialogue tree editor for games. 

It allows you to visually create dialog trees and export them as JSON files.
It is not a dialog system, but rather a way of visually creating and editing dialog assets. As such, it will work on any dialog system and game engine as long as you deserialize the JSON files on your end.
It supports **Speech phrase nodes** and **Player response nodes**.

The program is a single portable executable (~2.7MB) with no external dependencies.

Built with [ImGui](https://github.com/ocornut/imgui), [ImNodes](https://github.com/Nelarius/imnodes), and [SDL2](https://github.com/libsdl-org/SDL).

It currently only works on **Windows**.

<img src="https://github.com/user-attachments/assets/b8f0e885-0daa-4acf-a63a-2e986947150d" width="477" height="257"/>

## How to build

### Prerequisites
- C++ Compiler (GCC, Clang, or MSVC)
- CMake

### Building
Inside the cloned project run the following command:

```
git submodule update --init --recursive
```

Some dependancies are managed by [vcpkg](https://github.com/microsoft/vcpkg), therefore we need to bootstrap it and install the dependancies:
```
cd vcpkg
.\bootstrap-vcpkg.bat
vcpkg install sdl2 nlohmann-json
```
Alternatively, instead of running `vcpkg install sdl2 nlohmann-json`, if you are using Visual Studio 2022:
- Right click the **EasyDialogEditor** solution → Properties → vcpkg → Set `Use Vcpkg Manifest` to **Yes**

## FAQ (Frequently Asked Questions)

#### Q: Why does windows suspect a virus when opening *EasyDialogManager.exe* from Releases?
R: Windows will almost always give you that warning when running a downloaded executable. There is not much I can do, but you can always inspect the code and build from source.

#### Q: Does it work on Linux/MacOs ?
R: It currently only runs natively on Windows, but you may be able to run it on Linux using *Wine* (I didn't test it though).

## TODO
Planned upcoming features: 
- [ ] Event callback tags;
- [ ] Conditionals;
- [ ] Character names in Speech nodes;
- [ ] Simple dialogue player window;
- [ ] Mac and Linux support;
- [ ] Save/Load graph states;
- [ ] Undo/Redo

Feel free to fork and do whatever you want with the code. 
