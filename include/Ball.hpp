#pragma once
#include "Types.hpp"

class Ball {
private:
    Position pos; // Position of the ball
    float speed;
    int direction; // Direction of the ball

public:
    // Constructor for the Ball class
    Ball(Position startPos);

    // Getter and Setter
    Position getPosition() const;
    void setPosition(Position newPos);

    float getSpeed() const;
    void setSpeed(float newSpeed);
    int getDirection() const;

    // Method when ball kicked by striker() const;
    void kicked(int kickDirection, float initialSpeed = 3.0f);

    // Method update per-tick
    void update();
};