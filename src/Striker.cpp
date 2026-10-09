#include "Striker.hpp"
#include "Ball.hpp"
#include "Field.hpp"
#include <cmath>
#include <iostream> // for std::cout (used in think method)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor for the Striker class
Striker::Striker(Position startPos,  int startOrientation, float startSpeed)
    : Robot(startPos, startOrientation, startSpeed), 
    currentState(State::SEARCH_BALL), 
    lastKnownballPos({0.0f, 0.0f}), 
    ballDetected(false) {}

// getter and setter for currentState
State Striker::getCurrentState() const {return currentState;}
void Striker::setCurrentState(State newState) {currentState = newState;}

bool isBallInFOV(Position robotPos, int robotOrientation, Position ballPos) {
    float dx = ballPos.x - robotPos.x;
    float dy = ballPos.y - robotPos.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    if (distance > 2.5f) return false; // The ball is too far

    // Calculate the angle to the ball in degrees (0–359).
    float angleToBallRad = std::atan2(dy, dx); 
    float angleToBallDeg = angleToBallRad * (180.0f / M_PI); 
    angleToBallDeg = (fmod(angleToBallDeg, 360.0f) + 360.0f);
    angleToBallDeg = fmod(angleToBallDeg, 360.0f);

    // Calculate the angular difference relative to the robot's facing direction.
    float diff = std::abs(angleToBallDeg - robotOrientation);
    if (diff > 180.0f) diff = 360.0f - diff;

    // Enters the camera view if the difference is <= 45 degrees
    return diff <= 45.0f;
};

int chooseKickDirection(Position ballPos, int robotOrientation) {
    // Relative angle options
    int angleOptions[3] = {0, 45, -45}; // Straight, 45 degrees(diagonal up), 45 degrees right(diagonal down)

    for (int offset : angleOptions) {
       int targetAngle = (robotOrientation + offset + 360) % 360; // Normalize to 0-359
       float rad = targetAngle * (M_PI / 180.0f);

    //    Position simPos = ballPos;
    //    float currentSpeed = 3.0f; // Initial speed when kicked
       
       float distanceToGoalX = 4.5f - ballPos.x;
       float yAtGoalLine = ballPos.y + (distanceToGoalX * std::tan(rad));

       if (yAtGoalLine >= -1.25f && yAtGoalLine <= 1.25f) {
            return targetAngle;
       }
       // Ball movement simulation per tick
    //    while(currentSpeed > 0.0f) {
    //     simPos.x += currentSpeed * std::cos(rad);
    //     simPos.y += currentSpeed * std::sin(rad);

    //     if(simPos.x >= 4.5f && std::abs(simPos.y) <= 1.25f) {
    //         return targetAngle; // Return the target angle if the ball reaches the goal
    //     }

    //     currentSpeed -= 0.1f; // Deceleration of 1 m/tick
    //    }
    }

    return robotOrientation; // Default to current orientation if no valid kick direction found
}

// Implement the think method for the striker
void Striker::think(Ball& ball, const Field& field) {
    Position myPos = getPosition();

    if (isBallInFOV(myPos, getOrientation(), ball.getPosition())) {
        ballDetected = true;
        lastKnownballPos = ball.getPosition();
    } else {
        ballDetected = false;
    }

    switch (currentState) {
        case State::SEARCH_BALL: {
            // Logic for searching the ball
            if (ballDetected) {
                currentState = State::APPROACH_BALL; // Transition to approach ball state
            } else {
                setOrientation((getOrientation() + 90) % 360); // Rotate to search for the ball
            }
            break;
        }

        case State::APPROACH_BALL: {
            float dx = lastKnownballPos.x - myPos.x;
            float dy = lastKnownballPos.y - myPos.y;
            float distanceToBall = calculateDistance(lastKnownballPos);
            
            // Logic for approaching the ball
            if (myPos.x > lastKnownballPos.x + 0.2f) {
                if(std::abs(dy) < 0.5f){
                    setOrientation(dy >= 0 ? 270 : 90);
                } else {
                    setOrientation(180);
                }
            }
            else if (distanceToBall <= 0.7f) {
                currentState = State::ALIGN_TO_GOAL; // Transition to align to goal state
            } else {
                if (std::abs(dx) > std::abs(dy)) {
                    setOrientation(dx > 0 ? 0 : 180); // Move right or left
                } else {
                    setOrientation(dy > 0 ? 90 : 270); // Move up or down
                }
            }
            break;
        }
        case State::ALIGN_TO_GOAL:{
            // Logic for aligning to the goal
            setOrientation(0); // Align to face the goal (right direction)
            
            float dx = lastKnownballPos.x - myPos.x;
            float dy = std::abs(lastKnownballPos.y - myPos.y);

            if (dx > 0.0f && dx <= 0.8f && dy <= 0.4f) {
                currentState = State::KICK_BALL;
            } else {
                currentState = State::APPROACH_BALL;
            }
            break;
        }
        case State::KICK_BALL:{
            // Logic for kicking the ball
            int kickDirection = chooseKickDirection(lastKnownballPos, getOrientation());
            ball.kicked(kickDirection, 3.0f); // Kick the ball in the chosen direction

            currentState = State::SEARCH_BALL; // After kicking, go back to searching for the ball
            break;
        }
    }
}

void Striker::act() {
    // Striker only move if approaching ball
    if (currentState == State::APPROACH_BALL || currentState == State::SEARCH_BALL) {
        Robot::act();
    }
}