Instructions to build

$ mkdir build
$ cd build
$ cmake ..
$ make

Instructions to run

$ ./ahBakeTransform <input.exr> <output.png> <colorspace> <resize>

e.g. 
$ ./ahBakeTransform test.exr test_sRGB_ACES.png 3 1

Will produce a .png file that takes the scene-linear .exr file and outputs a baked ACES RRT+ODT suitable for LDR viewing on an sRGB device.
