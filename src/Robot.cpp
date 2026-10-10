#include "Robot.hpp"

#include <vector>

Robot::Robot(const Vec2& position, double headingDeg) {
    setPosition(position);
    setHeading(headingDeg);
    setSpeed(0.0);
}

void Robot::sense(const Ball& ball) {
    sensorData_ = sensor_.capture(position_, heading_, ball);
}

void Robot::setPosition(const Vec2& p) {
    if (!Field::inBounds(p)) {
        throw InvalidActionException("Posisi robot di luar lapangan");
    }
    position_ = p;
}

void Robot::setHeading(double deg) {
    if (!std::isfinite(deg)) throw InvalidActionException("Orientasi robot tidak valid");
    heading_ = MathUtils::normalizeAngle(deg);
}

void Robot::setSpeed(double s) {
    if (s < 0.0 || s > Const::MAX_ROBOT_SPEED + Const::EPS) {
        throw InvalidActionException("Kecepatan robot harus 0 - 0.5 m/tick");
    }
    speed_ = s;
}

Vec2 Robot::frontCell() const {
    return position_ + MathUtils::gridDirection(heading_) * Const::CELL_SIZE;
}

double Robot::distanceTo(const Vec2& target) const {
    return MathUtils::distance(position_, target);
}
double Robot::bearingTo(const Vec2& target) const {
    return MathUtils::bearing(position_, target);
}
double Robot::relativeBearingTo(const Vec2& target) const {
    return MathUtils::relativeBearing(position_, heading_, target);
}
double Robot::normalizeAngle(double deg) { return MathUtils::normalizeAngle(deg); }

Vec2 Robot::estimateBallWorldPosition(const SensorData& d) const {
    const double worldAngle = heading_ + d.ballBearing;
    const Vec2 p = position_ + MathUtils::unitVector(worldAngle) * d.ballDistance;
    return Field::snapToCell(p);
}

ActionCmd Robot::navigateTo(const Vec2& target, const Vec2* avoid) const {
    const Vec2 d = target - position_;
    const bool needX = std::fabs(d.x) > Const::CELL_SIZE / 2.0;
    const bool needY = std::fabs(d.y) > Const::CELL_SIZE / 2.0;
    if (!needX && !needY) return ActionCmd::wait();

    // Arah yang mendekatkan ke target; sumbu dengan sisa jarak terbesar didahulukan
    // (mencegah bolak-balik saat menghindari bola).
    std::vector<double> candidates;
    const double xHeading = d.x > 0 ? 0.0 : 180.0;
    const double yHeading = d.y > 0 ? 90.0 : 270.0;
    if (needX && needY) {
        if (std::fabs(d.y) > std::fabs(d.x) + Const::EPS) candidates = {yHeading, xHeading};
        else candidates = {xHeading, yHeading};
    } else if (needX) {
        candidates = {xHeading};
    } else {
        candidates = {yHeading};
    }

    auto isFree = [&](double h) {
        const Vec2 next = position_ + MathUtils::gridDirection(h) * Const::CELL_SIZE;
        if (!Field::inBounds(next)) return false;
        if (avoid && MathUtils::distance(next, *avoid) < Const::CELL_SIZE / 2.0) return false;
        return true;
    };

    bool found = false;
    double chosen = 0.0;
    for (double h : candidates) {
        if (isFree(h)) { chosen = h; found = true; break; }
    }
    if (!found) {  // terhalang bola -> geser ke samping dulu
        for (double off : {90.0, -90.0}) {
            const double h = MathUtils::normalizeAngle(candidates.front() + off);
            if (isFree(h)) { chosen = h; found = true; break; }
        }
    }
    if (!found) return ActionCmd::wait();

    const double diff = MathUtils::normalizeAngle(chosen - heading_);
    if (std::fabs(diff) > 1e-6) return ActionCmd::rotate(diff);
    return ActionCmd::move(Const::CELL_SIZE);
}
