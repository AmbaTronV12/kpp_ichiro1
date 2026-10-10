#include "Field.hpp"

#include <algorithm>

bool Field::inBounds(const Vec2& p) {
    return std::fabs(p.x) < Const::HALF_WIDTH && std::fabs(p.y) < Const::HALF_HEIGHT;
}

bool Field::isInGoal(const Vec2& p) {
    // Gawang = kolom petak paling kanan, dengan |y| <= 1.5 (lebar 3 m)
    return inBounds(p) && p.x > Const::HALF_WIDTH - Const::CELL_SIZE &&
           std::fabs(p.y) <= Const::GOAL_HALF_WIDTH;
}

Vec2 Field::cellCenter(int col, int row) {
    return {-Const::HALF_WIDTH + (col + 0.5) * Const::CELL_SIZE,
            -Const::HALF_HEIGHT + (row + 0.5) * Const::CELL_SIZE};
}

Vec2 Field::snapToCell(const Vec2& p) {
    int col = static_cast<int>(std::floor((p.x + Const::HALF_WIDTH) / Const::CELL_SIZE));
    int row = static_cast<int>(std::floor((p.y + Const::HALF_HEIGHT) / Const::CELL_SIZE));
    col = std::clamp(col, 0, Const::COLS - 1);
    row = std::clamp(row, 0, Const::ROWS - 1);
    return cellCenter(col, row);
}
