ABench Dependencies README
==========================

Current ABench dependencies are:

* FBX SDK 2018.1.1
* Google Test
* Vulkan


From these dependencies only Vulkan and Google Test are provided. Other dependencies must be installed manually and junction linked to this directory with following command:

```
mklink /J <link_name> <path_to_SDK>
```

Following links are required to build the project:

* "fbxsdk" - link to FBX SDK 2017.1 installed on the system (path where directories "include" and "lib" are visible)
