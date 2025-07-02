# modelLoader
loads and displays 3D models and their respective skeletal animations
=======

![GIF](lol.gif)

Figure 1 : zombie model from Half-Life

## clonig and building the repo
------------------

```bash
git clone https://github.com/0xf0ad/modelLoader.git && \
cd modelLoader && \
./build.sh
```

### dependencies
----------------

the repo is dependent on some libraries you should install from your package manager, those libraries are GLFW, GLM and Assimp on arch linux you can execute the folling command and you'll be good to go

```bash
sudo pacman -S glfw glm assimp
```

## using the tool
### Linux / other UNIX-like systems
-----------------
you can use the tool after clonning and compiling the repo by executing

```bash
./load <the path of 3D the model file> <the path to the animation if it existes>
```

if animations are embeded into the model file, animations could be loaded seperatly from runtime

```bash
./load a_guy_running.fbx
./load a_guy_running.md5mesh a_guy_running.md5anim #if the mesh and animation existe on seprate files, make sure to specify the mesh file first and animation one thereafter
```

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
