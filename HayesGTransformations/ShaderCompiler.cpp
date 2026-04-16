#define _CRT_SECURE_NO_WARNINGS
#include "ShaderCompiler.h"


ImageLoader imageLoader;
//compile shaders and store in provided program ID
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
        std::cout << "program linking successful for: " << vertName << " and " << fragName << std::endl;
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

//create buffer texture attachments for ambient occlusion
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

//create gbuffer texture attachments for deferred shading
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

//center and scale a mesh object
void ShaderCompiler::InitializeObject(cy::TriMesh& mesh, Object& object) {
    //compute the bounding box to center the object in local space
    mesh.ComputeBoundingBox();
    cy::Vec3f boundMin = mesh.GetBoundMin();
    cy::Vec3f boundMax = mesh.GetBoundMax();
    cy::Vec3f centerPoint;
    centerPoint.x = (boundMin.x + boundMax.x) / 2;
    centerPoint.y = (boundMin.y + boundMax.y) / 2;
    centerPoint.z = (boundMin.z + boundMax.z) / 2;

    //set object starting position, rotation, scale
    object.SetCenter(glm::vec3(centerPoint.x, centerPoint.y, centerPoint.z)); //centers object in local space
    object.SetPosition(0.0, 0.0f, 0.0f);
    object.SetScale(1.0f);
}

//load obj information into a cy::mesh
cy::TriMesh ShaderCompiler::LoadObjectFile(const char* fileName) {
    //create the mesh from obj data
    cy::TriMesh mesh;

    //check if mesh was opened correctly
    bool couldOpenPot = mesh.LoadFromFileObj(fileName);
    if (couldOpenPot) {
        std::cout << "loaded mesh from obj file!" << std::endl;
    }
    else {
        std::cout << "Could not load mesh from obj file!" << std::endl;
    }

    return mesh;
}

//create buffers for standard objects with obj meshes
void ShaderCompiler::CreateBuffers(ProgramInfo& programInfo, cy::TriMesh& mesh) {

    Object* object = programInfo.object;
    mesh = LoadObjectFile(object->objectFileName);

    //each mesh face has 3 associated vertices, store that number for later
    object->facesNumber = mesh.NF() * 3;

    //create a vector of vertices for vertex and normal buffering
    std::vector<Vertex> vertexData = std::vector<Vertex>(object->facesNumber);

    bool hasNormals = object->hasNormals;
    bool hasTexCoords = object->hasTexCoords;

    //takes position and normal values from vertices in the mesh, stores in vertexData vector above
    int vertexIndex = 0;
    for (int i = 0; i < mesh.NF(); i++) {
        // Get face indices for positions and normals
        const cy::TriMesh::TriFace& face = mesh.F(i);
        const cy::TriMesh::TriFace* faceNormal = hasNormals ? &mesh.FN(i) : nullptr;
        const cy::TriMesh::TriFace* faceTex = hasTexCoords ? &mesh.FT(i) : nullptr;

        for (int c = 0; c < 3; c++) {
            // store position data
            const auto& point = mesh.V(face.v[c]);
            vertexData[vertexIndex].position[0] = point.x;
            vertexData[vertexIndex].position[1] = point.y;
            vertexData[vertexIndex].position[2] = point.z;

            if (hasNormals) {
                // store normal data
                const auto& norm = mesh.VN(faceNormal->v[c]);
                vertexData[vertexIndex].normals[0] = norm.x;
                vertexData[vertexIndex].normals[1] = norm.y;
                vertexData[vertexIndex].normals[2] = norm.z;
            }

            if (hasTexCoords) {
                //store texture coordinates
                const auto& tex = mesh.VT(faceTex->v[c]);
                const float flippedUV = 1.0f - tex.y;
                vertexData[vertexIndex].texCords[0] = tex.x;
                vertexData[vertexIndex].texCords[1] = flippedUV;
            }

            vertexIndex++;
        }
    }

    //then generate texture image
   // if (object->hasTextures) { GenerateTextures(mesh, false); }

    //create buffer for holding mesh vertex data
    glGenBuffers(1, &programInfo.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, programInfo.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(Vertex), vertexData.data(), GL_STATIC_DRAW);

    //interpet position data
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

    if (hasNormals) {
        //interpret normal data
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Vertex::normals));
    }
    if (hasTexCoords) {

        //interpret texture data
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Vertex::texCords));
    }
}

//helper function for creating sampling kernals
float ShaderCompiler::Lerp(float a, float b, float f) {
    return a + f * (b - a);
}

//create random sampling kernels
void ShaderCompiler::CreateKernel() {
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

//creates and binds a texture, given a specified filename and uniform variable
//currenlty used for both diffuse and specularity
void ShaderCompiler::BindTexturesMTL(ProgramInfo& programInfo, const std::string& fileName, GLuint& texID, const GLchar* uniformName) {

    //create the texture variable
    glGenTextures(1, &texID);
    //this will hold the image raw data
    std::vector<unsigned char> image;

    //width and height of the texture image being loaded
    unsigned int textureWidth = 0;
    unsigned int textureHeight = 0;

    //load the image using lodePNG and output to image vector
    unsigned int imageSuccess = imageLoader.loadImage(image, textureWidth, textureHeight, fileName);

    //bind texture before filling with image data
    glActiveTexture(GL_TEXTURE0 + texID); //define unit zero, is also default unit
    glBindTexture(GL_TEXTURE_2D, texID);

    //create the actual image, fill with data from read file
    glTexImage2D(

        GL_TEXTURE_2D, //define as 2D texture type
        0, //at mipmap leveel 0 - highest resolution image
        GL_RGBA, //the internal formatting
        textureWidth, //image width
        textureHeight, //image height
        0, //border - this HAS to be 0
        GL_RGBA, //format - from image provided
        GL_UNSIGNED_BYTE, //data type = RGBA, RGBA, et cet in scanline format - 8 bits per channel
        &image[0] //pixel array data
    );

    //create mipmap levels
    //the order of the below function doesn't matter - 
    //we can generate mipmaps whenever as long as its before we use them and send to GPU!
    glGenerateMipmap(GL_TEXTURE_2D);

    //do trilinear filtering with mipmaps
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        //interpolates between two mipmap levels to avoid jittering
        GL_LINEAR_MIPMAP_LINEAR
    );

    //allows for magnification - when we get very close to texture,
    //individual texels can be larger than a single pixel!
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    //then we need to tell what to do when texture coord is outside 0-1

    //use repeated wrapping in t (vertical) direction
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );

    //use clamped wrapping in s (horizontal) direction
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    //setting the uniform sampler variable
    GLint sampler = glGetUniformLocation(programInfo.programID, uniformName);
    glUseProgram(programInfo.programID);
    glUniform1i(sampler, texID); //set to match the texture unit from glActiveTexture
    std::cout << "Successfully bound texture for: " << fileName << std::endl;
    programInfo.object->hasTextures = true;
}

//generates textures for given object
void ShaderCompiler::GenerateTextures(ProgramInfo& programInfo, cy::TriMesh& mesh, bool spec) {

    //load imageData with the pixel image data we get from the file
    int matNum = mesh.NM();
    std::cout << "Number of materials: " << matNum << std::endl;

    cy::TriMesh::Str diffuseTextureData;
    cy::TriMesh::Str specTextureData;

    for (int i = 0; i < matNum; i++) {
        const cy::TriMesh::Mtl& mat = mesh.M(0);
        diffuseTextureData = mat.map_Kd;
        if (spec) {
            specTextureData = mat.map_Ks;
        }
    }

    const std::string diffuseFileName(diffuseTextureData.data);
    std::cout << "DIFFUSE FILE NAME: " << diffuseFileName << std::endl;
    //test with PNG to see if this works!
    BindTexturesMTL(programInfo, diffuseFileName, programInfo.texIDDiffuse, "diffuseTex");


    if (spec) {
        const std::string specFileName(specTextureData.data);
        std::cout << "SPEC FILE NAME: " << specFileName << std::endl;
        //do the same thing now for the specular texture
        BindTexturesMTL(programInfo, specFileName, programInfo.texIDSpec, "specTex");
    }

}