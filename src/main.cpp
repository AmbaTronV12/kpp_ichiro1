#include <algorithm>
#include <iostream>
#include <memory>

#include "Config.hpp"
#include "Simulator.hpp"
#include "Striker.hpp"

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "config.txt";

    Config cfg;  // nilai default
    // argumen ke-2 (opsional): override jeda tick dalam ms, mis. ./soccer_sim config.txt 200
    try {
        cfg = Config::load(path);
        std::cout << "Konfigurasi dimuat dari '" << path << "'\n";
    } catch (const std::exception& e) {
        std::cerr << "[PERINGATAN] " << e.what() << " -> memakai konfigurasi default\n";
    }

    if (argc > 2) {
        try {
            cfg.tickDelayMs = std::max(0, std::stoi(argv[2]));
        } catch (const std::exception&) {
            std::cerr << "[PERINGATAN] jeda tick tidak valid, memakai " << cfg.tickDelayMs << " ms\n";
        }
    }

    try {
        if (MathUtils::distance(cfg.strikerPos, cfg.ballPos) < Const::CELL_SIZE / 2.0)
            throw std::runtime_error("Striker dan bola tidak boleh berada di petak yang sama");

        Ball ball(cfg.ballPos);
        auto striker = std::make_unique<Striker>(cfg.strikerPos, cfg.strikerHeading);
        Simulator sim(ball, std::move(striker), cfg.maxTicks);
        sim.run(cfg.stepMode, cfg.tickDelayMs);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
