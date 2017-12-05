ABench Dependencies README
==========================

Current ABench dependencies are:

* FBX SDK 2018.1.1
* FreeImage 3.17
* glslang
* Google Test
* Vulkan


From these dependencies only Vulkan and Google Test are provided. Other dependencies must be manually built or installed.

Installed modules
-----------------

Following links are required to build the project:

* "fbxsdk" - link to FBX SDK 2017.1 installed on the system (path where directories "include" and "lib" are visible)
* "freeimage" - link to FreeImage directory, a root directory of FreeImage DLL archive (inside should reside "Dist" directory). Rename "Dist/x32" to "Dist/x86" directory for compatibility. Can be skipped on Linux by installing libfreeimage development package from repositories.

Use mklink/ln to link dependencies:

```
# Windows:
mklink /J <link_name> <path_to_SDK>

# Linux:
ln -s <path_to_SDK> <link_name>
```

Git submodules
--------------

Some dependencies are downloaded in form of a Git submodule (ex. glslang).

Use `prepare_deps` script attached to the repo. It will automatically prepare everything that is needed.

The only requirement on Windows is adding MSBuild and CMake to PATH environment variable. In VS 2017, MSBuild is located typically in `C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin` directory.

On Linux, the script should work out of the box.
