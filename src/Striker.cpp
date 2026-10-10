#include "Striker.hpp"

#include <cstdlib>
#include <limits>
#include <queue>
#include <utility>

// ================================ Striker ===================================
Striker::Striker(const Vec2& position, double headingDeg)
    : Robot(position, headingDeg), state_(makeState(StateId::SEARCH)) {
    // Titik patroli: tiap titik + putaran 4 arah menutup blok 7x7 petak
    // (segitiga kamera kedalaman 3 petak). 3 kolom x 2 baris menutup seluruh lapangan.
    const int cols[] = {3, 10, 14};
    const int rows[] = {3, 8};
    for (int c : cols) waypoints_.push_back(Field::cellCenter(c, rows[0]));
    for (int i = 2; i >= 0; --i) waypoints_.push_back(Field::cellCenter(cols[i], rows[1]));
}

std::unique_ptr<StrikerState> Striker::makeState(StateId id) {
    switch (id) {
        case StateId::SEARCH: return std::make_unique<SearchState>();
        case StateId::APPROACH: return std::make_unique<ApproachState>();
        case StateId::ALIGN: return std::make_unique<AlignState>();
        case StateId::KICK: return std::make_unique<KickState>();
    }
    return std::make_unique<SearchState>();
}

void Striker::requestState(StateId id) { pending_ = id; }

std::string Striker::getStateName() const { return decidedState_; }

void Striker::updateBallMemory() {
    const SensorData& d = getSensorData();
    if (!d.ballVisible || cooldown_ > 0) return;  // saat cooldown bola masih bergerak
    const Vec2 est = estimateBallWorldPosition(d);
    if (!ballKnown_ || MathUtils::distance(est, ballPos_) > Const::CELL_SIZE / 2.0) {
        ballPos_ = est;
        plan_.valid = false;  // posisi bola berubah -> rencana lama tidak berlaku
    }
    ballKnown_ = true;
}

ActionCmd Striker::think() {
    updateBallMemory();
    for (int i = 0; i < 8; ++i) {
        const std::string used = state_->name();
        std::optional<ActionCmd> cmd = state_->think(*this, getSensorData());
        if (cmd) decidedState_ = used;  // state yang menghasilkan aksi tick ini
        if (pending_) {  // transisi state dilakukan SETELAH think() selesai (aman)
            state_ = makeState(*pending_);
            pending_.reset();
        }
        if (cmd) return *cmd;
    }
    return ActionCmd::wait();
}

Vec2 Striker::simulateKick(const Vec2& start, double dirDeg) {
    Ball sim(start);
    sim.kick(MathUtils::gridDirection(dirDeg));
    for (int guard = 0; sim.isMoving() && guard < 10; ++guard) sim.update();
    return sim.getPosition();
}

int Striker::kicksToGoal(const Vec2& start) {
    auto key = [](const Vec2& p) {
        const int c = static_cast<int>(std::floor((p.x + Const::HALF_WIDTH) / Const::CELL_SIZE));
        const int r = static_cast<int>(std::floor((p.y + Const::HALF_HEIGHT) / Const::CELL_SIZE));
        return r * Const::COLS + c;
    };
    std::vector<bool> seen(Const::COLS * Const::ROWS, false);
    std::queue<std::pair<Vec2, int>> q;
    q.push({start, 0});
    seen[key(start)] = true;

    while (!q.empty()) {
        const auto [pos, depth] = q.front();
        q.pop();
        if (Field::isInGoal(pos)) return depth;
        for (int a = 0; a < 8; ++a) {  // 8 arah tendangan (kelipatan 45 derajat)
            const double dir = 45.0 * a;
            // robot harus bisa berdiri di belakang bola (di dalam lapangan) dengan
            // hadap kardinal h dan offset k sehingga h + 45*k = arah tendangan
            bool canStand = false;
            for (double h : {0.0, 90.0, 180.0, 270.0}) {
                for (int k = -1; k <= 1; ++k) {
                    if (!MathUtils::nearlyEqual(MathUtils::normalizeAngle(h + 45.0 * k - dir), 0.0))
                        continue;
                    if (Field::inBounds(pos - MathUtils::gridDirection(h) * Const::CELL_SIZE))
                        canStand = true;
                }
            }
            if (!canStand) continue;
            const Vec2 end = simulateKick(pos, dir);
            if (MathUtils::distance(end, pos) < Const::EPS) continue;
            if (seen[key(end)]) continue;
            seen[key(end)] = true;
            q.push({end, depth + 1});
        }
    }
    return 99;
}

// Pilih (posisi tembak, arah hadap, offset tendangan) terbaik: minimalkan jumlah tendangan
// sampai gol (BFS atas fisika bola), lalu minimalkan jarak tempuh robot.
ShotPlan Striker::computePlan() const {
    ShotPlan best;
    double bestCost = std::numeric_limits<double>::max();

    for (double h : {0.0, 90.0, 180.0, 270.0}) {
        const Vec2 shootPos = ballPos_ - MathUtils::gridDirection(h) * Const::CELL_SIZE;
        if (!Field::inBounds(shootPos)) continue;

        for (int k = -1; k <= 1; ++k) {
            const Vec2 end = simulateKick(ballPos_, h + 45.0 * k);
            if (MathUtils::distance(end, ballPos_) < Const::EPS) continue;  // bola tidak bergerak

            const int kicks = Field::isInGoal(end) ? 1 : 1 + kicksToGoal(end);
            if (kicks >= 99) continue;  // jalan buntu (mis. bola masuk pojok)

            double cost = 100.0 * kicks;
            const Vec2 delta = shootPos - getPosition();
            cost += 0.1 * (std::fabs(delta.x) + std::fabs(delta.y)) / Const::CELL_SIZE;
            cost += 0.05 * std::fabs(MathUtils::normalizeAngle(h - getHeading())) / 90.0;
            cost += 0.01 * std::abs(k);  // sedikit lebih suka tendangan lurus

            if (cost < bestCost) {
                bestCost = cost;
                best = {true, shootPos, h, k};
            }
        }
    }
    return best;
}

// ============================== SearchState =================================
std::optional<ActionCmd> SearchState::think(Striker& s, const SensorData&) {
    if (s.cooldown_ > 0) {  // tunggu bola berhenti
        --s.cooldown_;
        return ActionCmd::wait();
    }
    if (s.ballKnown_) {
        s.requestState(StateId::APPROACH);
        return std::nullopt;
    }
    if (s.scanTurns_ < 3) {  // putar 4 arah untuk melihat sekeliling
        ++s.scanTurns_;
        return ActionCmd::rotate(90.0);
    }
    if (s.distanceTo(s.currentWaypoint()) < Const::CELL_SIZE / 2.0) {
        s.advanceWaypoint();  // sampai titik patroli -> scan lagi di titik berikutnya
        s.scanTurns_ = 0;
        return std::nullopt;
    }
    return s.navigateTo(s.currentWaypoint());
}

// ============================= ApproachState ================================
std::optional<ActionCmd> ApproachState::think(Striker& s, const SensorData&) {
    if (!s.ballKnown_) {
        s.requestState(StateId::SEARCH);
        return std::nullopt;
    }
    if (!s.plan_.valid) s.plan_ = s.computePlan();
    if (!s.plan_.valid) {  // tidak ada posisi tembak -> cari ulang
        s.ballKnown_ = false;
        s.requestState(StateId::SEARCH);
        return std::nullopt;
    }
    if (s.distanceTo(s.plan_.shootPos) < Const::CELL_SIZE / 2.0) {
        s.requestState(StateId::ALIGN);
        return std::nullopt;
    }
    return s.navigateTo(s.plan_.shootPos, &s.ballPos_);
}

// ============================== AlignState ==================================
std::optional<ActionCmd> AlignState::think(Striker& s, const SensorData& d) {
    const double diff = MathUtils::normalizeAngle(s.plan_.heading - s.getHeading());
    if (std::fabs(diff) > 1e-6) return ActionCmd::rotate(diff);

    // Arah sudah benar -> pastikan kamera melihat bola tepat di petak depan
    const bool ballInFront = d.ballVisible &&
                             MathUtils::nearlyEqual(d.ballDistance, Const::CELL_SIZE, 0.05) &&
                             std::fabs(d.ballBearing) < 1.0;
    if (ballInFront) {
        s.requestState(StateId::KICK);
        return std::nullopt;
    }
    s.ballKnown_ = false;  // memori ternyata salah -> cari ulang
    s.plan_.valid = false;
    s.requestState(StateId::SEARCH);
    return std::nullopt;
}

// =============================== KickState ==================================
std::optional<ActionCmd> KickState::think(Striker& s, const SensorData&) {
    const int offset = s.plan_.kickOffset;
    s.ballKnown_ = false;  // setelah ditendang, posisi bola lama tidak berlaku
    s.plan_.valid = false;
    s.cooldown_ = 3;       // bola berhenti paling lama 3 tick
    s.scanTurns_ = 0;
    s.requestState(StateId::SEARCH);
    return ActionCmd::kick(offset);
}
