#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "lodepng.h"
#include <string>
#include <iostream>

//this class handles all image loading
struct ImageLoader {

	//used when loading a standard image from png (such as a cube map)
	unsigned char* loadImageFromPNG(const char *fileName, int& width, int& height, int& colorChannels) {
		return stbi_load(fileName, &width, &height, &colorChannels, 0);
	}

	//used when loading a texture image from an MTL file (such as diffuse, spec textures)
    //use lodePNG to load an image from a filename
    unsigned int loadImage(std::vector<unsigned char>& image, unsigned int& width, unsigned int& height, const std::string& fileName) {
        unsigned int success = lodepng::decode(image, width, height, fileName);

        //check if the image was successfully loaded from the filename
        if (success == 0) {

            std::cout << "Image map was successfully loaded from mtl file:  " << fileName << std::endl;
        }
        else {
            std::cout << "Image map failed to load from mtl file!: " << fileName << std::endl;
        }

        return success;
    }

};
