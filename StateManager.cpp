#include "StateManager.h"

StateManager::StateManager()
  : stateIsValid(false) {
  // Initialize with default state
  savedState.color = 0xFFFFFF;  // White
  savedState.animation = ANIM_SOLID;
  savedState.animationSpeed = 50;
  savedState.brightness = 200;
  savedState.isAnimating = false;
}

void StateManager::saveLampState(const LEDController& ledController) {
#ifdef DEBUG
  Serial.println("[State] Saving lamp state...");
#endif
  
  // Capture all current lamp parameters
  savedState.color = ledController.getCurrentColor();
  savedState.animation = ledController.getCurrentAnimation();
  savedState.animationSpeed = ledController.getAnimationSpeed();
  savedState.brightness = ledController.getBrightness();
  savedState.isAnimating = ledController.isAnimating();
  
  stateIsValid = true;
  
#ifdef DEBUG
  Serial.print("[State] State saved - Color: 0x");
  Serial.print(savedState.color, HEX);
  Serial.print(" Animation: ");
  Serial.print(savedState.animation);
  Serial.print(" Speed: ");
  Serial.print(savedState.animationSpeed);
  Serial.print(" Brightness: ");
  Serial.println(savedState.brightness);
#endif
}

void StateManager::restoreLampState(LEDController& ledController) {
  if (!stateIsValid) {
#ifdef DEBUG
    Serial.println("[State] No valid state to restore");
#endif
    return;
  }
  
#ifdef DEBUG
  Serial.println("[State] Restoring lamp state...");
#endif
  
  // Restore all saved parameters
  ledController.setBrightness(savedState.brightness);
  
  if (savedState.isAnimating) {
    // Animation was running - restore and start it
    ledController.setAnimation(savedState.animation, savedState.animationSpeed);
    ledController.startAnimation();
    
#ifdef DEBUG
    Serial.println("[State] Animation restored and started");
#endif
  } else {
    // Animation was not running - restore color as solid
    ledController.setSolidColor(savedState.color);
    
#ifdef DEBUG
    Serial.print("[State] Solid color restored: 0x");
    Serial.println(savedState.color, HEX);
#endif
  }
}

bool StateManager::hasSavedState() const {
  return stateIsValid;
}
