#pragma once
#include "stb_image.h"
#include "lodepng.h"
#include <string>
#include <iostream>

//this class handles all image loading
struct ImageLoader {

	//used when loading a standard image from png (such as a cube map)
    unsigned char* loadImageFromPNG(const char*, int&, int&, int&);

	//used when loading a texture image from an MTL file (such as diffuse, spec textures)
    //use lodePNG to load an image from a filename
    unsigned int loadImage(std::vector<unsigned char>&, unsigned int&, unsigned int&, const std::string&);

};
