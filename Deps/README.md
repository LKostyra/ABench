ABench Dependencies README
==========================

Current ABench dependencies are:

* FBX SDK 2018.1.1
* FreeImage 3.17
* Google Test
* Vulkan


From these dependencies only Vulkan and Google Test are provided. Other dependencies must be installed manually and linked to this directory with following command:

```
# Windows:
mklink /J <link_name> <path_to_SDK>

# Linux:
ln -s <path_to_SDK> <link_name>
```

Following links are required to build the project:

* "fbxsdk" - link to FBX SDK 2017.1 installed on the system (path where directories "include" and "lib" are visible)
* "freeimage" - link to FreeImage directory, a root directory of FreeImage DLL archive (inside should reside "Dist" directory). Rename "Dist/x32" to "Dist/x86" directory for compatibility.

Git submodules
--------------

Some dependencies are downloaded in form of a Git submodule (ex. glslang).

You can either build them manually, or use `prepare_deps.bat` script attached to the repo. It will automatically prepare everything that is needed. The only requirement here is adding MSBuild and CMake to PATH environment variable. In VS 2017, MSBuild is located typically in `C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin` directory.
