#pragma once
#include "Ball.hpp"
#include "Types.hpp"

// Data hasil tangkapan kamera (relatif terhadap robot, BUKAN posisi global).
struct SensorData {
    bool ballVisible = false;
    double ballDistance = 0.0;  // m
    double ballBearing = 0.0;   // derajat, relatif arah hadap (kiri positif)
};

// Sensor kamera: segitiga alas 3.5 m, tinggi 1.5 m di depan robot.
// Robot HAS-A Sensor (composition).
class Sensor {
public:
    explicit Sensor(double base = Const::VIEW_BASE, double height = Const::VIEW_HEIGHT);

    bool isInView(const Vec2& robotPos, double headingDeg, const Vec2& point) const;
    SensorData capture(const Vec2& robotPos, double headingDeg, const Ball& ball) const;

private:
    void toLocal(const Vec2& robotPos, double headingDeg, const Vec2& point,
                 double& forward, double& left) const;
    double base_;
    double height_;
};
