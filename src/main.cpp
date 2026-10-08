#include "Simulator.hpp"
#include "Types.hpp"
#include <iostream>

int main() {
    float strikerX, strikerY;
    int strikerOrientation;
    float ballX, ballY;
    int maxTicks;

    // Input striker position
    std::cout << "Enter the Striker's starting position (X Y) [example: -2.0 0.0]: ";
    std::cin >> strikerX >> strikerY;

    // Input striker orientation
    std::cout << "Enter the Striker's facing direction (degrees) [0, 90, 180, 270]: ";
    std::cin >> strikerOrientation;

    // Input ball position
    std::cout << "Enter the initial position of the ball (X Y) [example: 0.0 0.0]: ";
    std::cin >> ballX >> ballY;

    // Input max ticks
    std::cout << "Enter the maximum tick limit [example: 30]: ";
    std::cin >> maxTicks;

    Position initialStrikerPosition = {strikerX, strikerY};
    Position initialBallPosition = {ballX, ballY};

    std::cout << "\n===========================================\n";
    std::cout << "          STARTING SIMULATION              \n";
    std::cout << "===========================================\n";

    // Initialize and Run the Simulator
    Simulator sim(initialStrikerPosition, strikerOrientation, initialBallPosition, maxTicks);
    sim.run();

    return 0;
}