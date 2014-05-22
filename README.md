viz
===

I'm sort of keeping a reboot here of my old graphics tools.  Feel free to fork or request pull or open issue.

![64gon in spin sample](https://raw.githubusercontent.com/bradgrantham/viz/master/spin_64gon.png)

Build requirements:
```
sudo port install glfw # last tests with "glfw @3.1.0_20140504_0+docs"
sudo port install boost # last tested with "boost @1.55.0_2+no_single+no_static+python27"
sudo port install assimp
sudo port install freeimage
```

I'm trying to stay with C++ and GL Core 3.2 for desktop apps and GLES 2 for mobile apps.  I'm using GLFW on desktop, not sure yet about mobile.

I plan to use FreeImage and ASSIMP to import models and images and save images.

Subdirectories:
* library - headers and implementations
* simple_spin - hardcoded vertex array polytope with trackball
  * Single program, vertex array, and list of triangles
  * Trackball demonstrated
  * Simple per-pixel lighting
  * Perhaps suitable for cloning to make simple geometry samples or tests
* spin - more flexible scene display code
  * Encapsulated program and geometry objects
  * Simple per-pixel lighting, target is more complex shading including texturing
  * Meant to be standalone viewer for models
  * Some example "spin" command lines
```
spin 256gon.builtin
spin 64gon.builtin
```
