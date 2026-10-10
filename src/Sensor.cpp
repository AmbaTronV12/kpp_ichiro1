#include "Sensor.hpp"

Sensor::Sensor(double base, double height) : base_(base), height_(height) {}

void Sensor::toLocal(const Vec2& robotPos, double headingDeg, const Vec2& point,
                     double& forward, double& left) const {
    const Vec2 rel = point - robotPos;
    const double h = MathUtils::toRadians(headingDeg);
    forward = rel.x * std::cos(h) + rel.y * std::sin(h);
    left = -rel.x * std::sin(h) + rel.y * std::cos(h);
}

bool Sensor::isInView(const Vec2& robotPos, double headingDeg, const Vec2& point) const {
    double f = 0.0, l = 0.0;
    toLocal(robotPos, headingDeg, point, f, l);
    if (f < Const::EPS || f > height_ + Const::EPS) return false;
    // Lebar setengah segitiga membesar linear terhadap jarak ke depan
    return std::fabs(l) <= f * (base_ / 2.0) / height_ + Const::EPS;
}

SensorData Sensor::capture(const Vec2& robotPos, double headingDeg, const Ball& ball) const {
    SensorData data;
    const Vec2 b = ball.getPosition();  // hanya Sensor (kamera) yang "melihat" bola
    if (!isInView(robotPos, headingDeg, b)) return data;

    double f = 0.0, l = 0.0;
    toLocal(robotPos, headingDeg, b, f, l);
    data.ballVisible = true;
    data.ballDistance = std::hypot(f, l);
    data.ballBearing = MathUtils::toDegrees(std::atan2(l, f));
    return data;
}
