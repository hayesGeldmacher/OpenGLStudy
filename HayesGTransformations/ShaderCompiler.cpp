#define _CRT_SECURE_NO_WARNINGS
#include "ShaderCompiler.h"

void ShaderCompiler::CompileShaders(const char* vertName, const std::string& fragName, GLuint& vaoID, GLuint& programID) {
    //manually compile vertex shader
    std::ifstream fVert(vertName);
    std::string file_contentsVert{ std::istreambuf_iterator<char>(fVert), std::istreambuf_iterator<char>() };
    //create char array of correct length, copy string into char array
    char* vsSource = new char[file_contentsVert.length() + 1];
    std::strcpy(vsSource, file_contentsVert.c_str());
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* sourceChar = vsSource;
    glShaderSource(vs, 1, &sourceChar, nullptr);
    glCompileShader(vs);

    //check vertex compilation success
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::VERTEX::COMPILATION_FAILED FOR:" << vertName <<
            infoLog << std::endl;
    }
    else {
        std::cout << "Vertex shader compilation successful for " << vertName << std::endl;
    }

    //compile fragment shader
    std::ifstream fFrag(fragName);
    std::string file_contentsFrag{ std::istreambuf_iterator<char>(fFrag), std::istreambuf_iterator<char>() };
    char* fsSource = new char[file_contentsFrag.length() + 1];
    std::strcpy(fsSource, file_contentsFrag.c_str());
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragChar = fsSource;
    glShaderSource(fs, 1, &fragChar, nullptr);
    glCompileShader(fs);

    //get frag shader compilation success
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        std::cout << "ERROR:SHADER::FRAG::COMPILATION_FAILED FOR: " << fragName <<
            infoLog << std::endl;
    }
    else {
        std::cout << "Fragment shader compilation successful for " << fragName << std::endl;
    }

    //create and link program
    programID = glCreateProgram();
    glAttachShader(programID, vs);
    glAttachShader(programID, fs);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cout << "ERROR: PROGRAM LINKING FAILED: " << infoLog << std::endl;
    }
    else {
        std::cout << "program linking successful" << std::endl;
    }

    //use program and delete shader objects
    glUseProgram(programID);
    glDeleteShader(vs);
    glDeleteShader(fs);

    //create vertex array object before the buffer,
    //stores the connections betwen a buffer and attributes for a particular object
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
}

void ShaderCompiler::CreateSSAOBuffer(int width, int height) {
    //creats the SSAO frame buffer object with color attachments

        //first create the frame buffer for storing AO color
        glGenFramebuffers(1, &AOFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, AOFBO);

        glGenTextures(1, &AOColorBuffer);
        glBindTexture(GL_TEXTURE_2D, AOColorBuffer);
        //set to red because AO is greyscale component, only need the one color channel
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AOColorBuffer, 0);

        //next create the frame buffer for blurring AO to remove random noise artifacts
        glGenFramebuffers(1, &AOBlurFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, AOBlurFBO);

        glGenTextures(1, &AOColorBufferBlur);
        glBindTexture(GL_TEXTURE_2D, AOColorBufferBlur);
        //also only need one color channel for ao blurring
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //set min and mag texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AOColorBufferBlur, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShaderCompiler::CreateDeferredBuffer(int width, int height) {
    //generate and bind the gbuffers
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // setting up position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); //creating the texture space
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //setting nearest neighbor filtering for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // setting nearest neighbor filtering for magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //clamps so we dont oversample
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // setting up normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // setting up specular and color buffer
    glGenTextures(1, &gColorSpec);
    glBindTexture(GL_TEXTURE_2D, gColorSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

    //position is 0 buffer, normal is 1 buffer, color is 2
    // define which color attachments will be used for rendering;
    unsigned int atttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, atttachments);

    //generate depth map
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    glGenerateMipmap(GL_TEXTURE_2D);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "warning! gbuffer did not finish compiling!" << std::endl;
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


float ShaderCompiler::Lerp(float a, float b, float f) {
    return a + f * (b - a);
}

void ShaderCompiler::CreateKernal() {
    //creates the hemisphere kernal for sampling points around AO frag

        //create sample kernal of points in hemisphere oriented along z tanget vector
        std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); //creates random floats between 0 and 1
        std::default_random_engine randGenerator; //declare instance of random num generator
        for (unsigned int i = 0; i < kernelNumber; ++i) {
            glm::vec3 sample(
                randomFloats(randGenerator) * 2.0f - 1.0f,
                randomFloats(randGenerator) * 2.0f - 1.0f,
                randomFloats(randGenerator)//dont offset z, would create sphere instead of hemisphere
            );

            //weigh points more heavily as they are closer to the center fragmnent
            float size = (float)i / kernelNumber;
            size = Lerp(0.1f, 1.0f, size * size);
            sample *= size;
            kernel.push_back(sample);
        }

        //create random rotation noise data
        std::vector<glm::vec3> AONoise;
        for (unsigned int i = 0; i < 16; i++) {
            glm::vec3 noise(
                randomFloats(randGenerator) * 2.0f - 1.0f,
                randomFloats(randGenerator) * 2.0f - 1.0f,
                0.0f); //leave z at zero to rotate around z axis
            AONoise.push_back(noise);
        }

        //create tiling 4x4 noise texture to overlay on the screen

        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        //fill texture with noise data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &AONoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}