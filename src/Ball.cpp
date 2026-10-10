#include "Ball.hpp"

#include "Field.hpp"

Ball::Ball(const Vec2& position) { setPosition(position); }

void Ball::setPosition(const Vec2& p) {
    if (!Field::inBounds(p)) {
        throw InvalidActionException("Posisi bola di luar lapangan");
    }
    position_ = p;
}

void Ball::kick(const Vec2& gridDir) {
    const bool validX = gridDir.x == -1.0 || gridDir.x == 0.0 || gridDir.x == 1.0;
    const bool validY = gridDir.y == -1.0 || gridDir.y == 0.0 || gridDir.y == 1.0;
    if (!validX || !validY || (gridDir.x == 0.0 && gridDir.y == 0.0)) {
        throw InvalidActionException("Arah tendangan tidak valid");
    }
    direction_ = gridDir;
    speed_ = Const::KICK_INITIAL_SPEED;
}

void Ball::update() {
    if (!isMoving()) return;

    // Pada tick ini bola menempuh 'speed_' meter = speed_/0.5 petak
    const int steps = static_cast<int>(std::lround(speed_ / Const::CELL_SIZE));
    for (int i = 0; i < steps; ++i) {
        const Vec2 next = position_ + direction_ * Const::CELL_SIZE;
        if (!Field::inBounds(next)) {  // menabrak batas lapangan
            speed_ = 0.0;
            return;
        }
        position_ = next;
        if (Field::isInGoal(position_)) {  // masuk gawang -> berhenti
            speed_ = 0.0;
            return;
        }
    }
    speed_ -= Const::BALL_DECELERATION;
    if (speed_ < Const::EPS) speed_ = 0.0;
}
