#pragma once


//struct for holding the positions, normals, and later tex coords for each vertex in a model
struct Vertex {

    Vertex() {
        position[0] = 0.0f;
        position[1] = 0.0f;
        position[2] = 0.0f;

        normals[0] = 0.0f;
        normals[1] = 0.0f;
        normals[2] = 0.0f;

        texCords[0] = 0.0f;
        texCords[1] = 0.0f;
    };

    //stores position data
    float position[3];
    //stores normal direction
    float normals[3];
    //stores texture UV coordinates
    float texCords[2];
};
