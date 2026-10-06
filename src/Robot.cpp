#include "Robot.hpp"
#include <cmath> // for std::sqrt and std::pow (used in calculateDistance)

Robot::Robot(float startspeed, int startOrientationn, Position startPos)
    : speed(startspeed), orientation(startOrientationn), pos(startPos) {}

Position Robot::getPosition() const { return pos; }
void Robot::setPosition(Position newPos) { pos = newPos; }

int Robot::getOrientation() const { return orientation; }
void Robot::setOrientation(int newOrientation) { orientation = newOrientation; }

float Robot::getSpeed() const { return speed; }
void Robot::setSpeed(float newSpeed) { speed = newSpeed; }

float Robot::calculateDistance(Position target) const {
    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    return std::sqrt(std::pow(dx, 2) + std::pow(dy, 2)); // Calculate Euclidean distance
}

void Robot::act() {
    if (orientation == 0){
        pos.x += speed; // Move right
    } else if (orientation == 90){
        pos.y += speed; // Move up
    } else if (orientation == 180){
        pos.x -= speed; // Move left
    } else if (orientation == 270){
        pos.y -= speed; // Move down
    }
}