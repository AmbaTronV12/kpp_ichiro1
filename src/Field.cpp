#include "Field.hpp"
#include <iostream>
#include <cmath>

Field::Field(float w, float h) : width(w), height(h) {
    cols = std::round(width / 0.5f); // Each column represents 0.5 meters
    rows = std::round(height / 0.5f); // Each row represents 0.5 meters
    clearGrid();
}

void Field::clearGrid() {
    grid = std::vector<std::string>(rows, std::string(cols, '.'));
}

bool Field::isInside(Position pos) const {
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;
    return (pos.x >= -halfW && pos.x <= halfW && pos.y >= -halfH && pos.y <= halfH);
}

bool Field::posToGrid(Position pos, int& outRow, int& outCol) const {
    if(!isInside(pos)) return false;

    outCol = static_cast<int>((pos.x + (width / 2.0f)) / 0.5f);
    outRow = static_cast<int>(((height / 2.0f - pos.y) / 0.5f));

    // Ensure matrix index limit
    if (outCol >= cols) outCol = cols - 1;
    if (outRow >= rows) outRow = rows - 1;

    return true;
}

void Field::drawObject(Position pos, char symbol) {
    int r, c;
    if(posToGrid(pos, r, c)) {
        grid[r][c] = symbol;
    }
}

void Field::drawGoal() {
    // Goal # is placed on the rightmost boundary (x = 4.5m)
    int goalCol = cols - 1;
    int startRow = rows / 3;
    int endRow = (2 * rows) / 3;

    for (int r = startRow; r <=endRow; ++r) {
        grid[r][goalCol] = '#';
    }
}

void Field::render() const {
    // Clear the terminal screen using ANSI escape codes
    std::cout << "\033[H\033[J";

    // Print top border line
    std::cout << "+" << std::string(cols, '-') << "+\n";

    for (int r = 0; r < rows; ++r) {
        std::cout << "|" << grid[r] << "|\n";
    }

    // Print bottom border line
    std::cout << "+" << std::string(cols, '-') << "+\n";
}

int Field::getCols() const { return cols; }
int Field::getRows() const { return rows; }