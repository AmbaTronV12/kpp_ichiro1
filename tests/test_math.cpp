// Unit test sederhana (Level 3) untuk matematika vektor, sensor, dan fisika bola.
// Kompilasi: g++ -std=c++17 -Iinclude tests/test_math.cpp src/Field.cpp src/Ball.cpp src/Sensor.cpp -o test_math
#include <iostream>

#include "Ball.hpp"
#include "Field.hpp"
#include "Sensor.hpp"
#include "Types.hpp"

static int g_fail = 0;
static int g_total = 0;
#define CHECK(cond)                                                      \
    do {                                                                 \
        ++g_total;                                                       \
        if (!(cond)) {                                                   \
            std::cerr << "FAIL (baris " << __LINE__ << "): " #cond "\n"; \
            ++g_fail;                                                    \
        }                                                                \
    } while (0)

using namespace MathUtils;

void testMath() {
    CHECK(nearlyEqual(distance({0, 0}, {3, 4}), 5.0));
    CHECK(nearlyEqual(normalizeAngle(190.0), -170.0));
    CHECK(nearlyEqual(normalizeAngle(-190.0), 170.0));
    CHECK(nearlyEqual(normalizeAngle(360.0), 0.0));
    CHECK(nearlyEqual(normalizeAngle(540.0), 180.0));
    CHECK(nearlyEqual(normalizeAngle(-180.0), 180.0));
    CHECK(nearlyEqual(bearing({0, 0}, {0, 1}), 90.0));
    CHECK(nearlyEqual(bearing({0, 0}, {-1, 0}), 180.0));
    CHECK(nearlyEqual(relativeBearing({0, 0}, 90.0, {1, 0}), -90.0));
    CHECK(gridDirection(45.0).x == 1.0 && gridDirection(45.0).y == 1.0);
    CHECK(gridDirection(180.0).x == -1.0 && gridDirection(180.0).y == 0.0);
    CHECK(gridDirection(-90.0).x == 0.0 && gridDirection(-90.0).y == -1.0);
}

void testField() {
    CHECK(Field::inBounds(Field::cellCenter(0, 0)));
    CHECK(!Field::inBounds({4.5, 0.0}));
    CHECK(Field::isInGoal(Field::cellCenter(17, 6)));    // y = 0.25
    CHECK(!Field::isInGoal(Field::cellCenter(17, 0)));   // y = -2.75 (di luar lebar gawang)
    CHECK(!Field::isInGoal(Field::cellCenter(16, 6)));   // bukan kolom gawang
    const Vec2 snapped = Field::snapToCell({0.3, 0.6});
    CHECK(nearlyEqual(snapped.x, 0.25) && nearlyEqual(snapped.y, 0.75));
}

void testSensor() {
    Sensor sensor;
    const Vec2 r = Field::cellCenter(9, 6);
    // Segitiga selalu 3 + 5 + 7 = 15 petak, untuk keempat arah hadap
    for (double heading : {0.0, 90.0, 180.0, -90.0}) {
        int count = 0;
        for (int c = 0; c < Field::cols(); ++c)
            for (int row = 0; row < Field::rows(); ++row)
                if (sensor.isInView(r, heading, Field::cellCenter(c, row))) ++count;
        CHECK(count == 15);
    }
    CHECK(sensor.isInView(r, 0.0, r + Vec2(0.5, 0.0)));    // petak depan
    CHECK(sensor.isInView(r, 0.0, r + Vec2(1.5, 1.5)));    // ujung alas
    CHECK(!sensor.isInView(r, 0.0, r + Vec2(0.5, 1.0)));   // terlalu miring dekat robot
    CHECK(!sensor.isInView(r, 0.0, r + Vec2(-0.5, 0.0)));  // di belakang
    CHECK(!sensor.isInView(r, 0.0, r + Vec2(2.0, 0.0)));   // lebih jauh dari 1.5 m

    Ball ball(r + Vec2(1.0, 0.5));
    SensorData d = sensor.capture(r, 0.0, ball);
    CHECK(d.ballVisible);
    CHECK(nearlyEqual(d.ballDistance, std::hypot(1.0, 0.5)));
    CHECK(d.ballBearing > 0.0);  // bola di sisi kiri
}

void testBall() {
    Ball b(Field::cellCenter(2, 6));
    b.kick({1.0, 0.0});
    b.update();  // 3 m = 6 petak
    CHECK(nearlyEqual(b.getPosition().x, Field::cellCenter(8, 6).x));
    CHECK(nearlyEqual(b.getSpeed(), 2.0));
    b.update();  // 2 m = 4 petak
    CHECK(nearlyEqual(b.getPosition().x, Field::cellCenter(12, 6).x));
    b.update();  // 1 m = 2 petak
    CHECK(nearlyEqual(b.getPosition().x, Field::cellCenter(14, 6).x));
    CHECK(!b.isMoving());
    b.update();  // sudah berhenti
    CHECK(nearlyEqual(b.getPosition().x, Field::cellCenter(14, 6).x));

    Ball g(Field::cellCenter(5, 6));  // 12 petak dari kolom gawang
    g.kick({1.0, 0.0});
    for (int i = 0; i < 3; ++i) g.update();
    CHECK(Field::isInGoal(g.getPosition()));

    Ball w(Field::cellCenter(16, 0));  // menabrak dinding
    w.kick({1.0, -1.0});
    w.update();
    CHECK(!w.isMoving());

    bool thrown = false;
    try { b.kick({0.0, 0.0}); } catch (const InvalidActionException&) { thrown = true; }
    CHECK(thrown);
    thrown = false;
    try { b.setPosition({10.0, 0.0}); } catch (const InvalidActionException&) { thrown = true; }
    CHECK(thrown);
}

int main() {
    testMath();
    testField();
    testSensor();
    testBall();
    std::cout << (g_total - g_fail) << "/" << g_total << " test lolos\n";
    return g_fail == 0 ? 0 : 1;
}
