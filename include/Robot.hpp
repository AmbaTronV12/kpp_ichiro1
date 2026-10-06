#pragma once
#include "Types.hpp"

class Robot {
protected:
    float speed;
    int orientation; // 0-360 degrees
    Position pos;

public:
    Robot(float startspeed, int startOrientation, Position startPos);

    virtual ~Robot() = default;

    //getter and setter
    Position getPosition() const;
    void setPosition(Position newPos);

    int getOrientation() const;
    void setOrientation(int newOrientation);

    float getSpeed() const;
    void setSpeed(float newSpeed);

    float calculateDistance(Position target) const;  //used to calculate distance between robot and target position

    virtual void think() = 0; //pure virtual function to be implemented by derived classes

    virtual void act();

};