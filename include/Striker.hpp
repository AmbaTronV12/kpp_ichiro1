#pragma once
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Robot.hpp"

// ----------------------- State Pattern (Level 3) ---------------------------
enum class StateId { SEARCH, APPROACH, ALIGN, KICK };

// Rencana tembakan: berdiri di shootPos, hadap heading, tendang dengan offset (-1/0/+1)
struct ShotPlan {
    bool valid = false;
    Vec2 shootPos;
    double heading = 0.0;
    int kickOffset = 0;
};

class Striker;

class StrikerState {
public:
    virtual ~StrikerState() = default;
    // Return nullopt = state berpindah/ingin dievaluasi ulang pada tick yang sama.
    virtual std::optional<ActionCmd> think(Striker& s, const SensorData& data) = 0;
    virtual std::string name() const = 0;
};

class SearchState : public StrikerState {
public:
    std::optional<ActionCmd> think(Striker& s, const SensorData& data) override;
    std::string name() const override { return "SEARCH_BALL"; }
};
class ApproachState : public StrikerState {
public:
    std::optional<ActionCmd> think(Striker& s, const SensorData& data) override;
    std::string name() const override { return "APPROACH_BALL"; }
};
class AlignState : public StrikerState {
public:
    std::optional<ActionCmd> think(Striker& s, const SensorData& data) override;
    std::string name() const override { return "ALIGN_TO_GOAL"; }
};
class KickState : public StrikerState {
public:
    std::optional<ActionCmd> think(Striker& s, const SensorData& data) override;
    std::string name() const override { return "KICK"; }
};

// ------------------------------- Striker -----------------------------------
class Striker : public Robot {
public:
    Striker(const Vec2& position, double headingDeg);

    ActionCmd think() override;
    std::string getStateName() const override;
    void requestState(StateId id);

private:
    friend class SearchState;
    friend class ApproachState;
    friend class AlignState;
    friend class KickState;

    void updateBallMemory();
    ShotPlan computePlan() const;
    // Hasil akhir bola (simulasi fisika mental) jika ditendang ke arah dirDeg dari 'start'
    static Vec2 simulateKick(const Vec2& start, double dirDeg);
    // Jumlah tendangan minimum (BFS) agar bola dari 'start' masuk gawang; 99 = mustahil
    static int kicksToGoal(const Vec2& start);
    Vec2 currentWaypoint() const { return waypoints_[waypointIdx_]; }
    void advanceWaypoint() { waypointIdx_ = (waypointIdx_ + 1) % waypoints_.size(); }
    static std::unique_ptr<StrikerState> makeState(StateId id);

    std::unique_ptr<StrikerState> state_;
    std::optional<StateId> pending_;
    std::string decidedState_ = "SEARCH_BALL";  // nama state yang memutuskan aksi tick ini

    bool ballKnown_ = false;   // memori bola (hasil estimasi dari kamera)
    Vec2 ballPos_;
    ShotPlan plan_;
    int cooldown_ = 0;         // menunggu bola berhenti setelah tendangan
    int scanTurns_ = 0;        // jumlah putaran 90 derajat saat mencari
    std::size_t waypointIdx_ = 0;
    std::vector<Vec2> waypoints_;
};
