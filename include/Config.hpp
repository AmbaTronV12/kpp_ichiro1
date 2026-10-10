#pragma once
#include <string>

#include "Types.hpp"

// Config (Level 3: File Konfigurasi). Dibaca dari config.txt
struct Config {
    Vec2 strikerPos{-3.75, -1.25};
    double strikerHeading = 0.0;
    Vec2 ballPos{0.25, 0.75};
    int maxTicks = 300;
    bool stepMode = false;
    int tickDelayMs = 500;  // jeda antar tick (ms) agar animasi bisa diikuti mata

    static Config load(const std::string& path);  // melempar std::runtime_error jika gagal
};
