#include "ActuatorManager.h"
#include <string.h>

ActuatorManager::ActuatorManager()
    : open_(false), closeAtMillis_(0), hasTimeout_(false) {
  lastSource_[0] = '\0';
}

void ActuatorManager::begin() {
  servo.setPeriodHertz(50);
  servo.attach(PIN_SERVO, 500, 2400);
  close();
}

void ActuatorManager::open(unsigned long durationSec, const char *sourceActuator) {
  servo.write(SERVO_ANGLE_OPEN);
  open_ = true;
  hasTimeout_ = durationSec > 0;
  closeAtMillis_ = hasTimeout_ ? millis() + (durationSec * 1000UL) : 0;
  strncpy(lastSource_, sourceActuator, sizeof(lastSource_) - 1);
  lastSource_[sizeof(lastSource_) - 1] = '\0';
}

void ActuatorManager::close() {
  servo.write(SERVO_ANGLE_CLOSED);
  open_ = false;
  hasTimeout_ = false;
  closeAtMillis_ = 0;
}

void ActuatorManager::update() {
  if (open_ && hasTimeout_ && millis() >= closeAtMillis_) {
    close();
  }
}

bool ActuatorManager::isOpen() const {
  return open_;
}

const char *ActuatorManager::lastSource() const {
  return lastSource_;
}
