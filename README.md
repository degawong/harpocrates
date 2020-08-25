
### config view tool
* copy `module/tools/harpocrates_mat.natvis` to 
`visual studio install path\Common7\Packages\Debugger\Visualizers`(`image-watch` is needed)
### build project
* (only for visual studio 2017 && later)
```cmake
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
```
