#include "Config.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "Field.hpp"

Config Config::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Tidak dapat membuka file konfigurasi: " + path);

    Config cfg;
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        const auto hash = line.find('#');  // komentar
        if (hash != std::string::npos) line.erase(hash);
        for (char& c : line) if (c == '=') c = ' ';

        std::istringstream iss(line);
        std::string key, valueStr;
        if (!(iss >> key)) continue;  // baris kosong
        const std::string where = " (baris " + std::to_string(lineNo) + ")";
        if (!(iss >> valueStr)) throw std::runtime_error("Nilai kosong untuk '" + key + "'" + where);

        double v = 0.0;
        try {
            v = std::stod(valueStr);
        } catch (const std::exception&) {
            throw std::runtime_error("Nilai bukan angka: '" + valueStr + "'" + where);
        }

        if (key == "striker_x") cfg.strikerPos.x = v;
        else if (key == "striker_y") cfg.strikerPos.y = v;
        else if (key == "striker_heading") cfg.strikerHeading = v;
        else if (key == "ball_x") cfg.ballPos.x = v;
        else if (key == "ball_y") cfg.ballPos.y = v;
        else if (key == "max_ticks") cfg.maxTicks = static_cast<int>(v);
        else if (key == "step_mode") cfg.stepMode = (v != 0.0);
        else if (key == "tick_delay_ms") cfg.tickDelayMs = static_cast<int>(v);
        else throw std::runtime_error("Key tidak dikenal: '" + key + "'" + where);
    }

    if (cfg.maxTicks <= 0) throw std::runtime_error("max_ticks harus > 0");
    if (cfg.tickDelayMs < 0) throw std::runtime_error("tick_delay_ms harus >= 0");
    if (std::fmod(std::fabs(cfg.strikerHeading), 90.0) > Const::EPS)
        throw std::runtime_error("striker_heading harus kelipatan 90 (0/90/180/270)");
    cfg.strikerPos = Field::snapToCell(cfg.strikerPos);  // posisi dipaksa ke pusat petak
    cfg.ballPos = Field::snapToCell(cfg.ballPos);
    return cfg;
}
