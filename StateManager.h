#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <cstdint>
#include "LEDController.h"

// Structure to hold lamp state for save/restore
struct LampState {
  uint32_t color;              // Current color in GRB format
  AnimationType animation;     // Animation type (SOLID, RAINBOW, PULSE, etc)
  uint16_t animationSpeed;     // Speed/timing of animation
  uint8_t brightness;          // Brightness level (0-255)
  bool isAnimating;            // Was animation running?
};

class StateManager {
public:
  StateManager();
  
  // Save current lamp state
  void saveLampState(const LEDController& ledController);
  
  // Restore previously saved lamp state
  void restoreLampState(LEDController& ledController);
  
  // Check if valid state is saved
  bool hasSavedState() const;
  
private:
  LampState savedState;
  bool stateIsValid;
};

#endif
