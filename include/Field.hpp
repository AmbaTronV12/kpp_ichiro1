#pragma once
#include "Types.hpp"
#include <vector>
#include <string>

class Field {
private:
    float width;
    float height;
    int cols;
    int rows;
    std::vector<std::string> grid; // Matrix ASCII character

public:
    // Constructor with default width and height
    Field(float w = 9.0f, float h = 6.0f);

    // Clearing grid with '.' character
    void clearGrid();

    // Validate if the ball is within the field boundaries
    bool isInside(Position pos) const;

    // Helper function to convert meter to grid index
    bool posToGrid(Position pos, int& outRow, int& outCol) const;

    // Marking characters on the grid (R, O, @, #)
    void drawObject(Position pos, char symbol);
    void drawGoal();

    // Displaying field in terminal per tick
    void render() const;

    // Getter size
    int getCols() const;
    int getRows() const;
};