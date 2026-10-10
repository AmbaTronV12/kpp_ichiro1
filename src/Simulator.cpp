#include "Simulator.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

namespace {
std::string directionName(double heading) {
    const double h = MathUtils::normalizeAngle(heading);
    if (MathUtils::nearlyEqual(h, 0.0)) return "EAST";
    if (MathUtils::nearlyEqual(h, 90.0)) return "NORTH";
    if (MathUtils::nearlyEqual(h, 180.0)) return "WEST";
    if (MathUtils::nearlyEqual(h, -90.0)) return "SOUTH";
    return "?";
}
}  // namespace

Simulator::Simulator(const Ball& ball, std::unique_ptr<Robot> robot, int maxTicks)
    : ball_(ball), robot_(std::move(robot)), maxTicks_(maxTicks) {
    if (!robot_) throw std::invalid_argument("Robot tidak boleh null");
    if (maxTicks_ <= 0) throw std::invalid_argument("maxTicks harus > 0");
}

void Simulator::run(bool stepMode, int tickDelayMs) {
    render("Kondisi Awal", ActionCmd::wait(), "");
    while (true) {
        if (stepMode) {
            std::cout << "\n[Enter untuk tick berikutnya] ";
            std::cin.get();
        } else if (tickDelayMs > 0) {  // mode animasi: tahan frame agar terbaca mata
            std::this_thread::sleep_for(std::chrono::milliseconds(tickDelayMs));
        }
        if (!tick()) break;
    }
    std::cout << "\n==============================\n";
    if (goal_) std::cout << "GOOOL! Bola masuk gawang pada tick " << tick_ << ".\n";
    else std::cout << "Waktu habis (" << maxTicks_ << " tick). Belum berhasil mencetak gol.\n";
}

bool Simulator::tick() {
    ++tick_;
    // 1. SENSE
    robot_->sense(ball_);
    // 2. THINK
    ActionCmd cmd = robot_->think();
    // 3. ACT (divalidasi simulator)
    std::string msg;
    try {
        executeAction(cmd);
    } catch (const InvalidActionException& e) {
        msg = std::string("[AKSI TIDAK VALID] ") + e.what() + " -> robot diam";
        robot_->setSpeed(0.0);
    }
    // 4. Fisika bola
    ball_.update();
    if (Field::isInGoal(ball_.getPosition())) goal_ = true;

    render("Tick " + std::to_string(tick_), cmd, msg);
    return !goal_ && tick_ < maxTicks_;
}

void Simulator::executeAction(const ActionCmd& cmd) {
    switch (cmd.type) {
        case ActionType::WAIT:
            robot_->setSpeed(0.0);
            break;

        case ActionType::MOVE_FORWARD: {
            if (cmd.value > Const::MAX_ROBOT_SPEED + Const::EPS)
                throw InvalidActionException("Kecepatan melebihi batas 0.5 m/tick");
            if (!MathUtils::nearlyEqual(cmd.value, Const::CELL_SIZE))
                throw InvalidActionException("Langkah harus tepat 1 petak (0.5 m)");
            const Vec2 target =
                robot_->getPosition() + MathUtils::gridDirection(robot_->getHeading()) * cmd.value;
            if (!Field::inBounds(target))
                throw InvalidActionException("Robot akan keluar lapangan");
            if (MathUtils::distance(target, ball_.getPosition()) < Const::CELL_SIZE / 2.0)
                throw InvalidActionException("Robot menabrak bola");
            robot_->setSpeed(cmd.value);
            robot_->setPosition(target);
            break;
        }

        case ActionType::ROTATE: {
            if (std::fabs(cmd.value) > 180.0 + Const::EPS ||
                std::fmod(std::fabs(cmd.value), 90.0) > Const::EPS)
                throw InvalidActionException("Rotasi harus kelipatan 90 derajat (maks 180)");
            robot_->setSpeed(0.0);
            robot_->setHeading(robot_->getHeading() + cmd.value);
            break;
        }

        case ActionType::KICK: {
            const int offset = static_cast<int>(std::lround(cmd.value));
            if (offset < -1 || offset > 1)
                throw InvalidActionException("Offset tendangan harus -1, 0, atau +1");
            if (MathUtils::distance(robot_->frontCell(), ball_.getPosition()) >
                Const::CELL_SIZE / 2.0)
                throw InvalidActionException("Tidak ada bola tepat di petak depan robot");
            robot_->setSpeed(0.0);
            ball_.kick(MathUtils::gridDirection(robot_->getHeading() + 45.0 * offset));
            break;
        }
    }
}

void Simulator::render(const std::string& label, const ActionCmd& cmd, const std::string& msg) const {
    const Vec2 rp = robot_->getPosition();
    const Vec2 bp = ball_.getPosition();

    std::ostringstream out;  // frame disusun dulu, dicetak sekali -> tidak berkedip
    out << std::fixed << std::setprecision(2);
    out << "\n=== " << label << " ===\n";
    out << "Robot : (" << rp.x << ", " << rp.y << ") hadap " << directionName(robot_->getHeading())
              << " | State: " << robot_->getStateName() << " | Aksi: " << cmd.toString() << "\n";
    out << "Bola  : (" << bp.x << ", " << bp.y << ") | kecepatan " << ball_.getSpeed()
              << " m/tick\n";
    if (!msg.empty()) out << msg << "\n";
    
    for (int row = Field::rows() - 1; row >= 0; --row) {  // baris atas = y positif
        for (int col = 0; col < Field::cols(); ++col) {
            const Vec2 c = Field::cellCenter(col, row);
            char sym = '.';
            if (Field::isInGoal(c)) sym = '#';
            if (robot_->getSensor().isInView(rp, robot_->getHeading(), c)) sym = '@';
            if (MathUtils::distance(c, bp) < Const::CELL_SIZE / 2.0) sym = 'O';
            if (MathUtils::distance(c, rp) < Const::CELL_SIZE / 2.0) sym = 'R';
            out << sym << (col < Field::cols() - 1 ? " " : "");
        }
        out << "\n";
    }
    std::cout << "\033[H\033[2J" << out.str() << std::flush;  // kursor ke pojok kiri atas, bersihkan, tampilkan
}
