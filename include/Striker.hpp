#pragma once
#include "Robot.hpp"
#include "Types.hpp"
#include "Ball.hpp"
#include "Field.hpp"

class Striker : public Robot {
private:
    State currentState; // Current state of the striker
    Position lastKnownballPos;
    bool  ballDetected;

public:
    // Constructor for the Striker class
    Striker(Position startPos, int startOrientation, float startSpeed);

    // Getter and setter for currentState
    State getCurrentState() const;
    void setCurrentState(State newState);

    void think(Ball& ball, const Field& field) override; // Implement the think method for the striker

    void act() override; // Override method from robot
};