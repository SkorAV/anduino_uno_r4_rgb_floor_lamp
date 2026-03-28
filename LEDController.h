#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Adafruit_NeoPixel.h>

// LED Strip Configuration
#define LED_PIN 5           // Data pin for WS2812B
#define LED_COUNT 84        // 3 strips x 28 LEDs
#define LED_BRIGHTNESS 200  // 0-255, default 200

// Animation types
enum AnimationType {
  ANIM_SOLID,
  ANIM_RAINBOW,
  ANIM_PULSE,
  ANIM_CHASE,
  ANIM_BLINK,
  ANIM_FADE,
  ANIM_SUNRISE,
  ANIM_SUNSET,
  ANIM_WAVES,
  ANIM_FLAME,
  ANIM_COUNT // Total count for cycling
};

class LEDController {
public:
  LEDController();
  ~LEDController();
  
  void begin();
  void update();
  
  // Color control
  void setSolidColor(uint8_t r, uint8_t g, uint8_t b);
  void setSolidColor(uint32_t color);
  void setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
  void setPixelColor(uint16_t index, uint32_t color);
  
  // Brightness control
  void setBrightness(uint8_t brightness);
  uint8_t getBrightness() const;
  
  // Power control
  void turnOn();
  void turnOff();
  bool isOn() const;
  
  // Animation control
  void setAnimation(AnimationType type, uint16_t speed = 50);
  void startAnimation();
  void stopAnimation();
  void freezeAnimation();
  bool isAnimating() const;
  bool isFrozen() const;
  
  // Cycling and adjustment
  void nextAnimation();
  void prevAnimation();
  void adjustSpeed(int16_t delta);
  void nextColorPreset();
  void setRandomAnimation();
  
  // State query methods
  uint32_t getCurrentColor() const;
  AnimationType getCurrentAnimation() const;
  uint16_t getAnimationSpeed() const;
  
  // Effects
  void clearStrip();
  void showStrip();
  
  // Utility
  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b);
  static void RGB2HSV(uint8_t r, uint8_t g, uint8_t b, uint8_t& h, uint8_t& s, uint8_t& v);
  static void HSV2RGB(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b);
  
private:
  Adafruit_NeoPixel strip;
  uint32_t currentColor;
  uint8_t currentBrightness;
  bool isPoweredOn;
  bool isRunning;
  bool isAnimationFrozen;
  
  AnimationType currentAnimation;
  uint16_t animationSpeed;
  uint32_t lastAnimationUpdate;
  uint16_t animationIndex;
  
  uint8_t colorPresetIndex;
  
  // Animation methods
  void updateRainbow();
  void updatePulse();
  void updateChase();
  void updateBlink();
  void updateFade();
  void updateSunrise();
  void updateSunset();
  void updateWaves();
  void updateFlame();
};

#endif
