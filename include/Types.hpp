#pragma once
// Types.hpp - konstanta, vektor 2D, helper matematika (DRY), aksi, dan exception.
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Const {
constexpr double PI = 3.14159265358979323846;
constexpr double EPS = 1e-6;

constexpr double CELL_SIZE = 0.5;           // 1 petak = 0.5 m
constexpr double FIELD_WIDTH = 9.0;
constexpr double FIELD_HEIGHT = 6.0;
constexpr double HALF_WIDTH = FIELD_WIDTH / 2.0;    // 4.5
constexpr double HALF_HEIGHT = FIELD_HEIGHT / 2.0;  // 3.0
constexpr int COLS = 18;                    // 9 m / 0.5 m
constexpr int ROWS = 12;                    // 6 m / 0.5 m

constexpr double GOAL_X = 4.5;              // gawang lawan di x = 4.5 m
constexpr double GOAL_HALF_WIDTH = 1.5;     // lebar gawang 3 m (|y| <= 1.5)

constexpr double MAX_ROBOT_SPEED = 0.5;     // m / tick
constexpr double KICK_INITIAL_SPEED = 3.0;  // m / tick
constexpr double BALL_DECELERATION = 1.0;   // m / tick

constexpr double VIEW_BASE = 3.5;           // alas segitiga kamera
constexpr double VIEW_HEIGHT = 1.5;         // tinggi segitiga kamera
}  // namespace Const

struct Vec2 {
    double x = 0.0;
    double y = 0.0;
    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(double s) const { return {x * s, y * s}; }
};

// ---------------------------------------------------------------------------
// Modul helper matematika: satu-satunya tempat rumus jarak, bearing, normalisasi.
// ---------------------------------------------------------------------------
namespace MathUtils {

inline double toRadians(double deg) { return deg * Const::PI / 180.0; }
inline double toDegrees(double rad) { return rad * 180.0 / Const::PI; }

inline bool nearlyEqual(double a, double b, double eps = Const::EPS) {
    return std::fabs(a - b) <= eps;
}

// Jarak Euclidean 2D
inline double distance(const Vec2& a, const Vec2& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

// Normalisasi sudut ke rentang (-180, 180]
inline double normalizeAngle(double deg) {
    double r = std::fmod(deg, 360.0);
    if (r > 180.0) r -= 360.0;
    else if (r <= -180.0) r += 360.0;
    return r;
}

// Bearing global (derajat) dari titik 'from' ke 'to'. 0 = sumbu +x, 90 = sumbu +y
inline double bearing(const Vec2& from, const Vec2& to) {
    return toDegrees(std::atan2(to.y - from.y, to.x - from.x));
}

// Bearing relatif terhadap arah hadap robot (positif = kiri / berlawanan jarum jam)
inline double relativeBearing(const Vec2& from, double headingDeg, const Vec2& to) {
    return normalizeAngle(bearing(from, to) - headingDeg);
}

inline Vec2 unitVector(double deg) {
    const double r = toRadians(deg);
    return {std::cos(r), std::sin(r)};
}

// Arah satu langkah grid (komponen -1/0/1) untuk kelipatan 45 derajat
inline Vec2 gridDirection(double deg) {
    const Vec2 u = unitVector(deg);
    return {std::round(u.x), std::round(u.y)};
}

}  // namespace MathUtils

// ---------------------------------------------------------------------------
// Aksi yang dikirim Robot ke Simulator (fase Act)
// ---------------------------------------------------------------------------
enum class ActionType { WAIT, MOVE_FORWARD, ROTATE, KICK };

struct ActionCmd {
    ActionType type = ActionType::WAIT;
    double value = 0.0;  // MOVE: jarak (m) | ROTATE: derajat | KICK: offset -1/0/+1

    static ActionCmd wait() { return {ActionType::WAIT, 0.0}; }
    static ActionCmd move(double meters) { return {ActionType::MOVE_FORWARD, meters}; }
    static ActionCmd rotate(double deg) { return {ActionType::ROTATE, deg}; }
    static ActionCmd kick(int offset) { return {ActionType::KICK, static_cast<double>(offset)}; }

    std::string toString() const {
        std::ostringstream os;
        os << std::fixed << std::setprecision(1);
        switch (type) {
            case ActionType::WAIT: return "WAIT";
            case ActionType::MOVE_FORWARD: os << "MOVE_FORWARD(" << value << " m)"; break;
            case ActionType::ROTATE: os << "ROTATE(" << value << " deg)"; break;
            case ActionType::KICK: os << "KICK(offset=" << static_cast<int>(value) << ")"; break;
        }
        return os.str();
    }
};

// Dilempar ketika sebuah aksi / nilai tidak valid menurut aturan simulasi
class InvalidActionException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
