#define STB_IMAGE_IMPLEMENTATION
#include "ImageLoader.h"

//used when loading a standard image from png (such as a cube map)
unsigned char* ImageLoader::loadImageFromPNG(const char* fileName, int& width, int& height, int& colorChannels) {
	return stbi_load(fileName, &width, &height, &colorChannels, 0);
}

//use lodePNG to load an image from a filename
unsigned int ImageLoader::loadImage(std::vector<unsigned char>& image, unsigned int& width, unsigned int& height, const std::string& fileName) {
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