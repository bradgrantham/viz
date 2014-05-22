viz
===

I'm sort of keeping a reboot here of my old graphics tools.  Feel free to fork or request pull or open issue.

![64gon in spin sample](https://raw.githubusercontent.com/bradgrantham/viz/master/spin_64gon.png)

Some guidelines:
* code is primarily in C++
* uses GLFW for windowing
* uses OpenMP for threading (if possible)
* uses Boost 1.55 for various Boost modules
  * shared_ptr 
* GL core ARB 3.2 or greater for graphics
* ASSIMP
* FreeImage

Subtrees:
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

Some example "spin" command lines
* spin 256gon.builtin
* spin 64gon.builtin
