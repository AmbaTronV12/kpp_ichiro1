#pragma once
#include "Types.hpp"
#include "Ball.hpp"
#include "Field.hpp"

class Robot {
protected:
    float speed;
    int orientation; // 0-360 degrees
    Position pos;

public:
    // Constructor for the Robot class
    Robot(Position startPos, int startOrientation, float startspeed);

    virtual ~Robot() = default;

    //getter and setter
    Position getPosition() const;
    void setPosition(Position newPos);

    int getOrientation() const;
    void setOrientation(int newOrientation);

    float getSpeed() const;
    void setSpeed(float newSpeed);

    float calculateDistance(Position target) const;  //used to calculate distance between robot and target position

    virtual void think(Ball& ball, const Field& field) = 0; //pure virtual function to be implemented by derived classes

    virtual void act();

};