#include "Striker.hpp"
#include <iostream> // for std::cout (used in think method)

// Constructor for the Striker class
Striker::Striker(Position startPos, float startspeed, int startOrientation)
    : Robot(startspeed, startOrientation, startPos), currentState(State::SEARCH_BALL) {}

// getter and setter for currentState
State Striker::getCurrentState() const {return currentState;}
void Striker::setCurrentState(State newState) {currentState = newState;}

// Implement the think method for the striker
void Striker::think() {
    switch (currentState) {
        case State::SEARCH_BALL:
            // Logic for searching the ball
            break;

        case State::APPORACH_BALL:
            // Logic for approaching the ball
            break;

        case State::ALIGN_TO_GOAL:
            // Logic for aligning to the goal
            break;

        case State::KICK_BALL:
            // Logic for kicking the ball
            break;
    }
}