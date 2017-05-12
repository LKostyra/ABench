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
