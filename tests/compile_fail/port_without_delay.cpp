/**
 * @file port_without_delay.cpp
 * @brief Must NOT compile: the chip's reset and conversion waits need the
 * port's DelayUs().
 *
 * EXPECT: DelayUs
 */
#include "ads9324.hpp"
#include "mock_ads9324.hpp"

class NoDelayPort : public ads9324_test::MockPortBase<NoDelayPort> {};

int main() {
  NoDelayPort port;
  ads9324::ADS9324<NoDelayPort> sensor(port);
  return static_cast<int>(sensor.EnsureInitialized().error);
}
