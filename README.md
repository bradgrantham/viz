viz
===

I'm sort of keeping a reboot here of my old graphics tools.  Feel free to fork or request pull or open issue.

Some guidelines:
* code is primarily in C++
* uses GLFW for windowing
* uses OpenMP for threading (if possible)
* uses Boost 1.55 for various Boost modules
* GL core ARB 3.2 or greater for graphics

Subtrees:
* library - headers and implementations
* simple_spin - hardcoded vertex array polytope with trackball
* spin - more flexible scene display code
