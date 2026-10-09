#include "Robot.hpp"
#include <cmath> // for std::sqrt and std::pow (used in calculateDistance)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Robot::Robot(Position startPos, int startOrientation,  float startspeed)
    : pos(startPos), orientation(startOrientation), speed(startspeed) {}

Position Robot::getPosition() const { return pos; }
void Robot::setPosition(Position newPos) { pos = newPos; }

int Robot::getOrientation() const { return orientation; }
void Robot::setOrientation(int newOrientation) { orientation = newOrientation; }

float Robot::getSpeed() const { return speed; }
void Robot::setSpeed(float newSpeed) { speed = newSpeed; }

float Robot::calculateDistance(Position target) const {
    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    return std::sqrt(dx * dx + dy * dy); // Calculate Euclidean distance
}

void Robot::act() {
    float rad = orientation * (M_PI / 180.0f);
    pos.x += speed * std::cos(rad);
    pos.y += speed * std::sin(rad);
}