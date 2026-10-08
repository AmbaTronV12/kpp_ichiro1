#include "Ball.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Ball::Ball(Position startPos)
    : pos(startPos), speed(0.0f), direction(0) {}

Position Ball::getPosition() const { return pos;}
void Ball::setPosition(Position newPos) {pos = newPos;}

float Ball::getSpeed() const {return speed;}
void Ball::setSpeed(float newSpeed) {speed = newSpeed;}

void Ball::kicked(int kickDirection, float initialSpeed) {
    direction = (kickDirection % 360 + 360) % 360;// Normalize direction to 0-359
    speed = initialSpeed;
}

void Ball::update() {
    if(speed > 0.0f) {
        float rad = direction * (M_PI / 180.0f); // Convert direction to radians for sin and cos function
        
        // Adjust the physical position according to speed and angular direction
        pos.x += speed * std::cos(rad);
        pos.y += speed * std::sin(rad);

        speed -= 1.0f; // Deceleration of 1 m/tick
        if (speed < 0.0f) {
            speed = 0.0f; // Ensure speed doesn't go negative
        }
    }
}
