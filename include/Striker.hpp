#pragma once
#include "Robot.hpp"
#include "Types.hpp"

class Striker : public Robot {
private:
    State currentState; // Current state of the striker

public:
    // Constructor for the Striker class
    Striker(Position startPos, float startspeed, int startOrientation);

    // Getter and setter for currentState
    State getCurrentState() const;
    void setCurrentState(State newState);

    void think() override; // Implement the think method for the striker
};