#pragma once
#include "Types.hpp"

// Ball: menyimpan posisi dan kecepatan bola. Fisika: v0 = 3 m/tick, turun 1 m/tick.
class Ball {
public:
    explicit Ball(const Vec2& position = Vec2(0.0, 0.0));

    const Vec2& getPosition() const { return position_; }
    void setPosition(const Vec2& p);               // tervalidasi (harus di dalam lapangan)
    double getSpeed() const { return speed_; }
    bool isMoving() const { return speed_ > Const::EPS; }

    // Tendang bola ke arah langkah grid (komponen -1/0/1, tidak boleh (0,0))
    void kick(const Vec2& gridDir);
    // Majukan fisika bola sebanyak 1 tick
    void update();

private:
    Vec2 position_;
    Vec2 direction_;
    double speed_ = 0.0;
};
