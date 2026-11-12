/**
 * @file daisy_chain_checked.cpp
 * @brief Must NOT compile: the daisy chain is out of scope, so a
 * `CheckedConfig` that asks for it is invalid.
 *
 * EXPECT: ConfigIsInvalid
 */
#include "ads9324_types.hpp"

int main() {
  constexpr ads9324::CheckedConfig kBoard{[] {
    ads9324::Config c{};
    c.daisy.enabled = true;
    return c;
  }()};
  return static_cast<int>(kBoard.Get().daisy.devices);
}
