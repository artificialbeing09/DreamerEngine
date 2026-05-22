# TODO

1. Shadows (better shaders)

* Add point lights and circular spotlights

3. Physics (rigid body) (partially completed)

6. Inbuilt animation system

7. Bones/rigs (would finally require a new shader)

8. Physics (soft body) (far off)

minor things

1. Global shadows (not just lighting)

2. Pixel specific stuff for UI

1. Studio default plugins for simple tools

3. Lua websockets and networking

5. load images from urls to textures

6. video processing n such

7. file io

11. Make stuff multithreaded

# Lost commits when merging into CMake project

```bash
* ad7fd2b (origin/safeinstance) Decreased shadow size
* 0337f42 Finished fixing events, and issues with SetParent.
* 97f785e Fixed instances (now safe)
* 036face First attempt
*   c5dca5a Merge branch 'master' of https://github.com/lightersmash1/LuaGE
|\
| * 34939ad Update README.md
| * 61259ad Update README.md
* | fc228a0 Did a smaller part of the stuff needed for multiple lights, had to remove all proper deletion of Instances from the project due to unsafe pointers. Will require a complete rewrite of the entire Instance system to work correctly.
* | 015229e Optimization to the object sorter.
|/
* 2322a0f Found essential bug with how parts are stored in the Render queue which was making them detach from themselves, creating ghost parts. Replaced RenderObjects array with a deque that stores both the Part and the render object itself, allowing for the same easy access as before. The advantage to a deque is that it's an actual linked list, and it allows for pointers to the stored value of things to never change.
* c5e4524 Shadow softness and first step toward multiple lights.
* 88550b0 Fixed a few lighting bugs as well as optimizing the shader's code for both cleanliness and performance.
* f7b6c91 Experimental start to shadows, very buggy.
*   f247538 Merge branch 'master' of https://github.com/lightersmash1/LuaGE
|\
| * 52ba0ef Update README.md
| * ca4145f Update README.md
* | ed8bbbb Changed, Destroying, ChildAdded, ChildRemoved, DescendantAdded, and DescendantRemoved events for all Instances. Stopped recursive parenting from working (parent being parented to its child), because it can lead to a program lock up for several things.
|/
*   a942d9b Merge branch 'master' of https://github.com/lightersmash1/LuaGE
|\
| * c16830e Update README.md
* | 0e1f7bf Added class inheritance to events because it was missing.
|/
*   7bfa6cc Merge branch 'master' of https://github.com/lightersmash1/LuaGE
|\
| * 54b8968 Update README.md
* | 5421043 Added KeyDown, KeyUp, WindowFocused, and WindowUnfocused, which are part of the Input service. Added parameter passing to event firings.
|/
*   6060593 Merge branch 'master' of https://github.com/lightersmash1/LuaGE
|\
| * e6c0606 Add TODO list to the readme.md
* | 0e9da8b Event disconnections, vector bug fix
|/
* 46794f8 Created and finished many events for TextObjects. Restructured Graphics, and cleared warnings.
* 0d15339 Note
* 482b30b Finish event system and textures are now more optimized with bugs fixed.
* ec276f3 2D Optimization and first part of event system
* 24dbd36 Finish up UI and Text
* b76a17a Text
* dbb2c0b Warning silencing.
* 3ac3ce8 Resolved all warnings and made lib inclusions specific to _WIN32
* cbae89d Moved 2D stuff to Graphics/2D/...
* 74763fa Add WIP 2D stuff
* 9cbb105 Moved stuff around some more
* a406ed6 Expanded out shader.h, made standard names in Graphics, moved 3D engine stuff around to be cleaner.
* 4b03e76 Added transparency
* b55369a Moved Scheduler loop to main graphics loop due to pretty much all crashes being caused by race conditions
* 740b333 Various Optimizations
* 574d379 Optimizations with Parts and Textures
* dc1f076 Added skyboxes, services (Scene and Sky), organized stuff, bug fixes etc.
* 8033969 Setting instance properties to nil is now possible. Added Test suite to test Lua execution and do stuff. Added Texture object to World (it now functions under Part.h)
* 390c911 Add sphere.obj
* 14cc8b8 Relocate external files to "Engine" folder, now textures will be loaded from that folder. Added a new get_descendants filesystem utility in utils.h
* 7dcd032 AddTexture and Texture::Initialize are now functional.
* dba034d Fragment/vertex shader now have a complete texture indexing system (ignoring materials). Texture.h is getting some work done.
* 90aa2d8 Changed name of fragment and vertex shader to allow for syntax highlighting.
* 18755a4 Cleaning up of Texture.h, now added texture ids to object data.
* 08c3b10 Added more stb libraries - Added Texture.h for handling texture actions etc, to be used as a util.
* 4dcf963 Add stb_image.h
* 328a6d2 Add sphere.obj
* 8175769 Add Teapot Obj (sphere.obj)
* 1774574 Re-add broken cube physics code that didn't work into a header to be used as an example.
* 1cbdc0c Add namespaces to (almost) all headers stuff.
* 70e2e8b Bug fixes and cleanups with Scheduler and Frame Rate
* 853c008 Update README.md
* 58414a5 Moved functions in main.cpp to dedicated headers.
* c4a2b57 1. You can now index an instance with the name of another instance to get a child of that instance with that name (E.G. Part.Part2 or game.Part).
* 7455d88 Bug fix: yielding lua functions can now return stuff
* 1194398 Add Scheduler for Lua so that multiple scripts can now run (not in parallel, but in sequence in between yields like sleeps etc).
* f30c0b7 Move unused code for defining a new shape as a type of render object into the function definition for defining a new shape to be used as example code.
* b3f2473 Add project files.
* 2c65e2f Add .gitattributes, .gitignore, and README.md.
(END)
```

