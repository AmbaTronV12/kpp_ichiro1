#include "Simulator.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

#ifndef M_PI
#define M_PI 3.1459265358979323846
#endif

Simulator::Simulator(Position strikerPos, int strikerOrient, Position ballPos, int maxTicks)
    : striker(strikerPos, 0.5f, strikerOrient), ball(ballPos), currentTick(0), maxTicks(maxTicks) {}

void Simulator::renderFOV() {
    Position rPos = striker.getPosition();
    int rOrient = striker.getOrientation();
    float rad = rOrient * (M_PI / 180.0f);

    // Loop for @ display
    for (int step = 1; step <= 3; ++step) {
        float forwardDist = step * 0.5f;

        for (int side = -step; side <= step; ++side) {
            float sideDist = side * 0.5f;

            // Coordinate transformation based on the robot's direction (0, 90, 180, 270)
            Position fovPos;
            fovPos.x = rPos.x + forwardDist * std::cos(rad) - sideDist * std::sin(rad);
            fovPos.y = rPos.y + forwardDist * std::sin(rad) + sideDist * std::cos(rad);

            if (field.isInside(fovPos)) {
                field.drawObject(fovPos, '@');
            }
        }
    }
}

bool Simulator::checkGoal() const {
    Position bPos = ball.getPosition();
    // A goal is scored if the ball passes x = 4.5m within the vertical range of the goalposts (y = -1.25m to 1.25m)
    return (bPos.x >= 4.5f && std::abs(bPos.y) <= 1.25f);
}

void Simulator::run() {
    std::string statusLog = "Running Simulation";
    while (currentTick < maxTicks) {
        currentTick++;

        // Cleaning frame from previous frame
        field.clearGrid();

        // Calling think method from striker to decide state
        striker.think(ball, field);

        // calling act method from striker and update method from ball to act based on the state and updating ball position if kicked 
        striker.act();
        ball.update();

        // Checking for goal
        if (checkGoal()) {
            statusLog = "GOAL! STRIKER SCORED!";

            // Frame when goal happen
            field.drawGoal(); 
            renderFOV();
            field.drawObject(ball.getPosition(), 'O' ); 
            field.drawObject(striker.getPosition(), 'R'); 
            field.render();

            std::cout << "Tick: " << currentTick
                  << " | Striker: (" << striker.getPosition().x << ", " << striker.getPosition().y << ")"
                  << " | Ball: (" << ball.getPosition().x << ", " << ball.getPosition().y << ")\n";
            std::cout << "GOAL! STRIKER SCORED!\n\n";
            break;
        }

        // Check if the ball has gone out of bounds
        if (!field.isInside(ball.getPosition())) {
            statusLog = "The ball is out of bounds! Respawn ball in center (0,0)";
            ball.setPosition({0.0f, 0.0f}); // Respawn on the center of the field
            ball.kicked(0, 0.0f); // Stop the ball's speed

        }

        field.drawGoal(); // Drawing '#' for goal
        renderFOV(); // Drawing '@' for camera sensor
        field.drawObject(ball.getPosition(), 'O' ); // Drawing 'O' for ball position
        field.drawObject(striker.getPosition(), 'R'); // Drawing 'R' for striker position

        field.render();
        std::cout << "Tick: " << currentTick
                  << " | Striker: (" << striker.getPosition().x << ", " << striker.getPosition().y << ")"
                  << " | Ball: (" << ball.getPosition().x << ", " << ball.getPosition().y << ")\n";

        std::cout << "Status: " << statusLog << "\n";
        // Hold the screen for 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}