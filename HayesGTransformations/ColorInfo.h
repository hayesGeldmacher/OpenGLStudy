#pragma once

//simple struct for tracking state of R,G,B values in for mesh teapot color animation
struct Color {

    //0-1
    float value;

    //whether value is going up or down in animation
    bool goingUp;

    //whether value is animating
    bool active;

    //simple constructor
    Color() {
        value = 0;
        goingUp = true;
        active = false;
    }

    //elaborate constructor
    Color(float value, bool goingUp, bool active) {
        this->value = value, this->goingUp = goingUp, this->active = active;
    }

    //sets active status to inverse, flips animation on or off
    void FlipActive() {
        active = !active;
    }

};