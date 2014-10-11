viz
===

I'm sort of keeping a reboot here of my old graphics tools.  Feel free to fork or request pull or open issue.

![64gon in spin sample](https://raw.githubusercontent.com/bradgrantham/viz/master/spin_64gon.png)
![sub.obj converted to TriSrc in spin sample](https://raw.githubusercontent.com/bradgrantham/viz/master/sub.png)
![Buddha TriSrc in spin sample](https://raw.githubusercontent.com/bradgrantham/viz/master/buddha.png)

Build requirements:
```
sudo port install glfw # last tests with "glfw @3.1.0_20140504_0+docs"
sudo port install boost # last tested with "boost @1.55.0_2+no_single+no_static+python27"
sudo port install freeimage # last tested with "freeimage @3.16.0_1"
sudo port install assimp
```

Subdirectories:
* library - headers and implementations
* simple_spin - hardcoded vertex array polytope with trackball
  * Single program, vertex array, and list of triangles
  * Trackball demonstrated
  * Simple per-pixel lighting
  * Perhaps suitable for cloning to make simple geometry samples or tests
* spin - more flexible scene display code
  * Encapsulated program and geometry objects
  * Simple per-pixel lighting including diffuse color texturing
  * Meant to be standalone viewer for models
  * Some example "spin" command lines:
```
    spin 256gon.builtin
    spin 64gon.builtin
```

To build Doxygen documentation, ``cd docs``, then *either:*
* doxywizard (on MacOS, ``port install doxygen +wizard``), load docs/doxyfile, run
* run doxygen directly: ``doxygen doxyfile``
