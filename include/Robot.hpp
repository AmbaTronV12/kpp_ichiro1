#pragma once
#include <string>

#include "Ball.hpp"
#include "Field.hpp"
#include "Sensor.hpp"
#include "Types.hpp"

// Robot: abstract class. Atribut private, diakses lewat getter/setter tervalidasi.
// Siklus: sense() -> think() [pure virtual] -> Simulator menjalankan aksi (Act).
class Robot {
public:
    Robot(const Vec2& position, double headingDeg);
    virtual ~Robot() = default;

    // Sense: kamera menangkap bola. Robot tidak pernah membaca posisi bola langsung.
    void sense(const Ball& ball);
    // Think: wajib di-override oleh subclass
    virtual ActionCmd think() = 0;
    virtual std::string getStateName() const { return "IDLE"; }

    const Vec2& getPosition() const { return position_; }
    double getHeading() const { return heading_; }
    double getSpeed() const { return speed_; }
    void setPosition(const Vec2& p);
    void setHeading(double deg);
    void setSpeed(double s);

    const Sensor& getSensor() const { return sensor_; }
    const SensorData& getSensorData() const { return sensorData_; }
    Vec2 frontCell() const;  // petak tepat di depan robot

protected:
    // Helper navigasi (DRY): dipakai semua subclass
    double distanceTo(const Vec2& target) const;
    double bearingTo(const Vec2& target) const;
    double relativeBearingTo(const Vec2& target) const;
    static double normalizeAngle(double deg);
    Vec2 estimateBallWorldPosition(const SensorData& data) const;
    // Jalan 4 arah (grid) ke target; 'avoid' = petak yang tidak boleh diinjak (mis. bola)
    ActionCmd navigateTo(const Vec2& target, const Vec2* avoid = nullptr) const;

private:
    Vec2 position_;
    double heading_ = 0.0;
    double speed_ = 0.0;
    Sensor sensor_;
    SensorData sensorData_;
};
