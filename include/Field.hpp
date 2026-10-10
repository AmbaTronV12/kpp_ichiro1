#pragma once
#include "Types.hpp"

// Field: aturan geometri lapangan 9 m x 6 m (grid 18 x 12 petak, 0.5 m per petak).
// Titik tengah (0,0). Pusat petak berada di x = -4.25, -3.75, ..., 4.25.
class Field {
public:
    static int cols() { return Const::COLS; }
    static int rows() { return Const::ROWS; }

    static bool inBounds(const Vec2& p);
    static bool isInGoal(const Vec2& p);          // petak '#' di sisi kanan, |y| <= 1.5
    static Vec2 cellCenter(int col, int row);      // col 0..17 (kiri->kanan), row 0..11 (bawah->atas)
    static Vec2 snapToCell(const Vec2& p);         // bulatkan ke pusat petak terdekat (clamp)
};
