#pragma once
#include "Striker.hpp"
#include "Ball.hpp"
#include "Field.hpp"

class Simulator {
private:
    Field field;
    Ball ball;
    Striker striker;
    int currentTick;
    int maxTicks;

    // Helper for rendering @
    void renderFOV();

    // Helper for checking goal
    bool checkGoal() const;

public:
    Simulator(Position strikerPos, int strikerOrient, Position ballPos,  int maxTicks = 30);

    // Main function for game loop
    void run();
};