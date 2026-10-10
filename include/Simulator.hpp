#pragma once
#include <memory>
#include <string>

#include "Ball.hpp"
#include "Field.hpp"
#include "Robot.hpp"

// Simulator: pengelola game loop berbasis tick (1 tick = 1 detik).
// Tiap tick: Sense -> Think -> Act (validasi) -> fisika bola -> render.
class Simulator {
public:
    Simulator(const Ball& ball, std::unique_ptr<Robot> robot, int maxTicks);

    void run(bool stepMode = false, int tickDelayMs = 0);
    bool tick();  // true jika simulasi masih lanjut

    bool isGoal() const { return goal_; }
    int getTickCount() const { return tick_; }

private:
    void executeAction(const ActionCmd& cmd);  // bisa melempar InvalidActionException
    void render(const std::string& label, const ActionCmd& cmd, const std::string& msg) const;

    Field field_;
    Ball ball_;
    std::unique_ptr<Robot> robot_;
    int tick_ = 0;
    int maxTicks_;
    bool goal_ = false;
};
