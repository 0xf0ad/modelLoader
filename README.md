# modelLoader
a program that loads 3D models 
=======

## clonig the repo
------------------

```bash
git clone https://github.com/hadsitewa3r/OpenGL.git && \
cd modelLoader && \
make
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
-----------------
you can use the tool after clonning and compiling the source code by executing

```bash
./load [the path of 3D the model file] [the path to the animation if there is any]
```

some file format containe the model and the animation at the same time that require you o type the same path twice if you want to access to the skeletal animation or once if you only want to view the 3D model contained in that file

you actially can access the tool from the therminal without 'cd'ing into the git directory if you install it and you can lunch it by

```bash
load [the path of the 3D model file] [the path to the animation if there is any]
```

(for now the animation is not yat implemented so you can only view 3D models at T-Pose)
(dont try to run the program whitout running a display server, it wont work, I tried it :) )

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
|Z                     |switch to normal mode      |
|X                     |switch to wireframe mode   |
|H                     |render the front faces     |
|J                     |render the back faces      |
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
