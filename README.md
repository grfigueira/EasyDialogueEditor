# EasyDialogueEditor

**EasyDialogueEditor** is a lightweight dialogue tree editor for games. 

It allows you to visually create dialogue trees and export them as JSON files.
It is not a dialogue system, but rather a way of visually creating and editing dialogue assets. As such, it will work on any dialogue system and game engine as long as you deserialize the JSON files on your end.
It supports **Speech phrase nodes** and **Player response nodes**.

The program is a single portable executable (~2.7MB) with no external dependencies.

Built with [ImGui](https://github.com/ocornut/imgui), [ImNodes](https://github.com/Nelarius/imnodes), and [SDL2](https://github.com/libsdl-org/SDL).

It currently only runs natively on **Windows**.

![image](https://github.com/user-attachments/assets/4f938ab9-cd14-4ea7-a7e9-adef4aa8483d)


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

#### Q: Why does Windows flag EasyDialogueManager.exe from Releases as a potential virus?
R: Windows often gives that warning about downloaded executables, especially from unknown developers. This is common and there is not much I can do. You can always review the source code and compile it yourself if you have concerns.

#### Q: Is it compatible with Linux or macOS?
R: The program currently only runs natively on Windows. It should work on Linux if you run it with Wine.

## TODO
Planned upcoming features: 
- [x] Event callback tags;
- [x] Save/Load graph states;
- [ ] Undo/Redo
- [ ] Character names in Speech nodes;
- [ ] Conditionals;
