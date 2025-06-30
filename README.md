# modelLoader
a program that loads 3D models 
=======

## clonig the repo
------------------

```bash
git clone https://github.com/0xf0ad/modelLoader.git && \
cd modelLoader && \
make
```

or prefferebly compile with CMake using the build script

```bash
cd modelLoader
./build.sh
```

if you want to access the tool as a command from the tharminal you can add

```bash
sudo make install
```

after you install the tool you or done with it if didn't install it you can clean the directory from binaries that way you can recompile it if you want to re-use it and cleanning up will not uninstall the tool from your computer if you installed it

```bash
make clean
```

if you installed it and want to uninstall it you can do that by executing

```bash
sudo rm /usr/bin/load
```

### dependencies
----------------

the repo is dependent on some librarys you should install from your package manager,those librarys are GLFW GLM Assimp on arch linux you can execute the folling command to install them

```bash
sudo pacman -S glfw glm assimp
```

## using the tool
### Linux / other UNIX-like systems
-----------------
you can use the tool after clonning and compiling the source code by executing

```bash
./load <the path of 3D the model file> <the path to the animation if existed>
```

if the model contain animations you can load it without them by speciffying the model path at the first argument, or with animation by re-entring the model file's path twice for example

```bash
./load ~/Documents/a_guy_running.fbx #to load the 3D model without animations
./load ~/Documents/a_guy_running.fbx ~/Documents/a_guy_running.fbx #to load the 3D model with their animating
```

the reason it is like that is for some files mainly .mdl and .md5 for IDteck and GldSrc and Source engines separate the 3D models and animations on several files


you actially can access the tool from the therminal without 'cd'ing into the git directory if you install it and you can lunch it by

```bash
load <the path of the 3D model file> <the path to the animation if there is any>
```

(dont try to run the program whitout running a display server, it wont work, I tried it :) )

### Windows
installing gentoo and building this repo is far easier than building it with windows
this is how you compile that project to windows using linux or using MinGW on windows

you should first compile extenal libraries
- glfw (download it from the official website and extract the .a file from the MinGW folder and move to /usr/x86_64-w64-mingw32/lib/ or the path to your libraries)
- assimp (you should clone the official assimp repo and build it using MinGW(good luck doing that you will need it a lot))
- place the assimp's dll on the same directory of you other libraries and build this project using
```bash
./build.sh -DCMAKE_SYSTEM_NAME=Windows
```
- now gather the dlls (libassimp-5.dll and libgcc_s_seh-1.dll and libstdc++-6.dll and libwinpthread-1.dll) to your local directory
- try to run it, it may or may not work (it did for me and stopt doing that)

## keybinding
-------------

|Key                   |action or event            |
|----------------------|---------------------------|
|holding MOUSE1        |rotate the camera          |
|W                     |move Forward               |
|A                     |move Righ                  |
|S                     |move Backward              |
|D                     |move Left                  |
|LEFT_SHIFT            |slowing movement           |
|ESCAPE                |exit the application       |

you can change the binding by modifying the souce code (suckless phelo) by modifing the processInput function on the bottom of main.cpp

(I propably i will emplement the UI, and when I do I will remove some keyBindings) 

## file extetion supported
--------------------------

* Collada                  ( .dae, .xml )
* Blender                  ( .blend )
* Biovision BVH            ( .bvh )
* 3D Studio Max 3DS        ( .3ds )
* 3D Studio Max ASE        ( .ase )
* Wavefront Object         ( .obj )
* Stanford Polygon Library ( .ply )
* AutoCAD DXF              ( .dxf )
* IFC-STEP                 ( .ifc )
* Neutral File Format      ( .nff )
* Sense8 WorldToolkit      ( .nff )
* Valve Model              ( .smd, .vta )
* Quake I                  ( .mdl )
* Quake II                 ( .md2 )
* Quake III                ( .md3 )
* Quake 3 BSP              ( .pk3 )
* RtCW                     ( .mdc )
* Doom 3                   ( .md5mesh, .md5anim, .md5camera )
* DirectX X                ( .x )
* Quick3D                  ( .q3o, .q3s )
* Raw Triangles            ( .raw )
* AC3D                     ( .ac )
* Stereolithography        ( .stl )
* Autodesk DXF             ( .dxf )
* Irrlicht Mesh            ( .irrmesh, .xml )
* Irrlicht Scene           ( .irr, .xml )
* Object File Format       ( .off )
* Terragen Terrain         ( .ter )
* 3D GameStudio Model      ( .mdl )
* 3D GameStudio Terrain    ( .hmp )
* Ogre                     ( .mesh.xml, .skeleton.xml, .material )
* Milkshape 3D             ( .ms3d )
* LightWave Model          ( .lwo )
* LightWave Scene          ( .lws )
* Modo Model               ( .lxo )
* CharacterStudio Motion   ( .csm )
* Stanford Ply             ( .ply )

## library used

* glad : a library to load OpenGL (http://glad.dav1d.de/)
* glfw : a firmware to commenicate with the graphics card driver (https://www.glfw.org/)
* glm : a library that support leniar algebra (https://glm.g-truc.net/)
* stb_image : a library to load images with variaus formats (https://github.com/nothings/stb/)
* assimp : a library to load 3D models with variaus formats (https://github.com/assimp/assimp/)
* ImGui : a library to reander a Graphical User Interface (https://github.com/ocornut/imgui/)
