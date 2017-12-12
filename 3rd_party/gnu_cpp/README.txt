This folder contains Minimal Embedded C++ support for the GNU-ARM toolchain.
The provided file mini_cpp.cpp eliminates the need for linking bulky C++
libraries for new/delete and static constructors/destructors.

Additionally, the files no_heap.cpp and sbrk.c provide dummy definitions
for malloc(), calloc(), and free() functions. These files should be used only
if you don't need the standard heap (which is highy recommended in real-time
embedded systems).



