This folder contains Minimal Embedded C++ support for the GNU g++ C++ toolset.
The provided file mini_cpp.cpp eliminates the need for linking bulky C++
libraries for new/delete and static constructors/destructors.

Additionally, the file no_heap.cpp provides dummy definitions for malloc(),
calloc(), and free() functions. This file should be used only if you don't
need the standard heap (which is highy recommended in real-time embedded
systems).



