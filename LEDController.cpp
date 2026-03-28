#include "LEDController.h"

LEDController::LEDController()
  : strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800),
    currentColor(0xFF0000),
    currentBrightness(LED_BRIGHTNESS),
    isPoweredOn(true),
    isRunning(false),
    isAnimationFrozen(false),
    currentAnimation(ANIM_SOLID),
    animationSpeed(50),
    lastAnimationUpdate(0),
    animationIndex(0),
    colorPresetIndex(0) {
}

LEDController::~LEDController() {
}

void LEDController::begin() {
  strip.begin();
  strip.setBrightness(currentBrightness);
  strip.show();
  
#ifdef DEBUG
  Serial.println("[LED] LED Controller initialized");
  Serial.print("[LED] Strip: ");
  Serial.print(LED_COUNT);
  Serial.println(" LEDs");
#endif
}

void LEDController::update() {
  if (!isPoweredOn || !isRunning || isAnimationFrozen) {
    return;
  }
  
  uint32_t now = millis();
  if (now - lastAnimationUpdate >= animationSpeed) {
    // POLITE UPDATE: Check if IR is currently receiving (active-low)
    // Pin 2 is the IR receiver pin.
    static uint32_t lastIrActivity = 0;
    
    // 1. Initial check
    if (digitalRead(2) == LOW) {
      lastIrActivity = now;
    }
    
    // 2. Pre-flight check: Wait a tiny bit and check again to catch a burst starting
    // NEC leader pulse is 9ms, so checking over 500us is safe.
    delayMicroseconds(250);
    if (digitalRead(2) == LOW) lastIrActivity = now;
    delayMicroseconds(250);
    if (digitalRead(2) == LOW) lastIrActivity = now;

    // 3. Silence threshold: Wait 100ms after ANY activity before updating LEDs.
    // A full NEC command is ~67ms. 100ms covers the command and the first gap.
    if (now - lastIrActivity < 100) {
      // IR activity detected recently. Defer LED update to keep interrupts free.
      return;
    }

    lastAnimationUpdate = now;
    
    switch (currentAnimation) {
      case ANIM_RAINBOW:
        updateRainbow();
        break;
      case ANIM_PULSE:
        updatePulse();
        break;
      case ANIM_CHASE:
        updateChase();
        break;
      case ANIM_BLINK:
        updateBlink();
        break;
      case ANIM_FADE:
        updateFade();
        break;
      case ANIM_SUNRISE:
        updateSunrise();
        break;
      case ANIM_SUNSET:
        updateSunset();
        break;
      case ANIM_WAVES:
        updateWaves();
        break;
      case ANIM_FLAME:
        updateFlame();
        break;
      case ANIM_SOLID:
      default:
        break;
    }
    
    yield();
    strip.show();  // ~4-5ms blocking call
    yield();
  }
}

void LEDController::setSolidColor(uint8_t r, uint8_t g, uint8_t b) {
  currentColor = Color(r, g, b);
  currentAnimation = ANIM_SOLID;
  isRunning = false;
  isAnimationFrozen = false;
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, currentColor);
  }
  strip.show();
}

void LEDController::setSolidColor(uint32_t color) {
  currentColor = color;
  currentAnimation = ANIM_SOLID;
  isRunning = false;
  isAnimationFrozen = false;
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, currentColor);
  }
  strip.show();
}

void LEDController::setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
  if (index < strip.numPixels()) {
    strip.setPixelColor(index, Color(r, g, b));
  }
}

void LEDController::setPixelColor(uint16_t index, uint32_t color) {
  if (index < strip.numPixels()) {
    strip.setPixelColor(index, color);
  }
}

void LEDController::setBrightness(uint8_t brightness) {
  if (brightness < 5) brightness = 5; // Prevent lossy color data washout
  currentBrightness = brightness;
  strip.setBrightness(brightness);
  strip.show();
}

uint8_t LEDController::getBrightness() const {
  return currentBrightness;
}

void LEDController::turnOn() {
  isPoweredOn = true;
  strip.show();
}

void LEDController::turnOff() {
  isPoweredOn = false;
  clearStrip();
}

bool LEDController::isOn() const {
  return isPoweredOn;
}

void LEDController::setAnimation(AnimationType type, uint16_t speed) {
  currentAnimation = type;
  animationSpeed = speed;
  animationIndex = 0;
  isAnimationFrozen = false;
  lastAnimationUpdate = millis();
}

void LEDController::startAnimation() {
  isRunning = true;
}

void LEDController::stopAnimation() {
  isRunning = false;
}

bool LEDController::isAnimating() const {
  return isRunning;
}

void LEDController::clearStrip() {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}

void LEDController::showStrip() {
  strip.show();
}

uint32_t LEDController::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void LEDController::RGB2HSV(uint8_t r, uint8_t g, uint8_t b, uint8_t& h, uint8_t& s, uint8_t& v) {
  uint8_t maxc = max(r, max(g, b));
  uint8_t minc = min(r, min(g, b));
  v = maxc;
  
  if (minc == maxc) {
    h = 0;
    s = 0;
    return;
  }
  
  s = 255 * (maxc - minc) / maxc;
  
  if (maxc == r) {
    h = 43 * (g - b) / (maxc - minc);
  } else if (maxc == g) {
    h = 85 + 43 * (b - r) / (maxc - minc);
  } else {
    h = 171 + 43 * (r - g) / (maxc - minc);
  }
}

void LEDController::HSV2RGB(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
  if (s == 0) {
    r = g = b = v;
    return;
  }
  
  uint8_t region = h / 43;
  uint8_t remainder = (h % 43) * 6;
  
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
  
  switch (region) {
    case 0:
      r = v; g = t; b = p;
      break;
    case 1:
      r = q; g = v; b = p;
      break;
    case 2:
      r = p; g = v; b = t;
      break;
    case 3:
      r = p; g = q; b = v;
      break;
    case 4:
      r = t; g = p; b = v;
      break;
    default:
      r = v; g = p; b = q;
      break;
  }
}

void LEDController::updateRainbow() {
  uint16_t pixelHue = (animationIndex * 65536L / strip.numPixels()) % 65536;
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    uint16_t hue = (pixelHue + i * 65536L / strip.numPixels()) % 65536;
    strip.setPixelColor(i, strip.ColorHSV(hue));
  }
  
  animationIndex++;
  if (animationIndex >= 256) {
    animationIndex = 0;
  }
}

void LEDController::updatePulse() {
  // Smooth pulsing effect using simple math instead of sin
  uint8_t brightness = animationIndex < 50 
    ? (animationIndex * 255 / 50)  // fade in
    : (255 - (animationIndex - 50) * 255 / 50);  // fade out
  
  strip.setBrightness(brightness);
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, currentColor);
  }
  
  animationIndex++;
  if (animationIndex >= 100) {
    animationIndex = 0;
  }
}

void LEDController::updateChase() {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    if (i == animationIndex) {
      strip.setPixelColor(i, currentColor);
    } else {
      strip.setPixelColor(i, 0);
    }
  }
  
  animationIndex++;
  if (animationIndex >= strip.numPixels()) {
    animationIndex = 0;
  }
}

void LEDController::updateBlink() {
  if (animationIndex < 50) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, currentColor);
    }
  } else {
    clearStrip();
  }
  
  animationIndex++;
  if (animationIndex >= 100) {
    animationIndex = 0;
  }
}

void LEDController::updateFade() {
  uint8_t brightness = 255 * animationIndex / 100;
  strip.setBrightness(brightness);
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, currentColor);
  }
  
  animationIndex++;
  if (animationIndex >= 100) {
    animationIndex = 0;
  }
}

void LEDController::updateSunrise() {
  // Sunrise: from dark red (0, 0) to orange to yellow to white
  float progress = (float)animationIndex / 255.0f;
  uint8_t r, g, b;
  
  if (progress < 0.5f) {
    // Red to Orange
    r = 255;
    g = progress * 2.0f * 165.0f;
    b = 0;
  } else {
    // Orange to Yellow/White
    r = 255;
    g = 165 + (progress - 0.5f) * 2.0f * (255 - 165);
    b = (progress - 0.5f) * 2.0f * 255;
  }
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(r, g, b));
  }
  
  animationIndex++;
  if (animationIndex > 255) animationIndex = 0;
}

void LEDController::updateSunset() {
  // Sunset: from yellow to orange to red to purple/dark
  float progress = (float)animationIndex / 255.0f;
  uint8_t r, g, b;
  
  if (progress < 0.5f) {
    // Yellow to Orange/Red
    r = 255;
    g = 255 - progress * 2.0f * 200.0f;
    b = progress * 2.0f * 50.0f;
  } else {
    // Red to Purple/Dark
    r = 255 - (progress - 0.5f) * 2.0f * 200.0f;
    g = 55 - (progress - 0.5f) * 2.0f * 55.0f;
    b = 50 + (progress - 0.5f) * 2.0f * 100.0f;
  }
  
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Color(r, g, b));
  }
  
  animationIndex++;
  if (animationIndex > 255) animationIndex = 0;
}

void LEDController::updateWaves() {
  // Sea Waves: blues and cyans moving
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    float angle = (float)animationIndex * 0.1f + (float)i * 0.5f;
    uint8_t b = 155 + sin(angle) * 100;
    uint8_t g = 100 + cos(angle * 0.8f) * 100;
    strip.setPixelColor(i, Color(0, g, b));
  }
  
  animationIndex++;
}

void LEDController::updateFlame() {
  // Flame: flickering reds and oranges
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    uint8_t flicker = random(0, 50);
    uint8_t r = 255 - flicker;
    uint8_t g = 80 - flicker / 2;
    uint8_t b = 0;
    
    // Add some random "sparks"
    if (random(0, 100) > 98) {
      r = 255; g = 200; b = 50;
    }
    
    strip.setPixelColor(i, Color(r, g, b));
  }
  // No index needed for random flicker, but we keep it for consistency
  animationIndex++;
}

uint32_t LEDController::getCurrentColor() const {
  return currentColor;
}

AnimationType LEDController::getCurrentAnimation() const {
  return currentAnimation;
}

uint16_t LEDController::getAnimationSpeed() const {
  return animationSpeed;
}

void LEDController::freezeAnimation() {
  if (isRunning) {
    isAnimationFrozen = !isAnimationFrozen;
  }
}

bool LEDController::isFrozen() const {
  return isAnimationFrozen;
}

void LEDController::nextAnimation() {
  int next = (int)currentAnimation + 1;
  if (next >= ANIM_COUNT) next = 1; // Skip SOLID
  setAnimation((AnimationType)next, animationSpeed);
  startAnimation();
}

void LEDController::prevAnimation() {
  int prev = (int)currentAnimation - 1;
  if (prev <= 0) prev = ANIM_COUNT - 1; // Skip SOLID
  setAnimation((AnimationType)prev, animationSpeed);
  startAnimation();
}

void LEDController::adjustSpeed(int16_t delta) {
  int32_t newSpeed = (int32_t)animationSpeed + delta;
  if (newSpeed < 10) newSpeed = 10;
  if (newSpeed > 1000) newSpeed = 1000;
  animationSpeed = (uint16_t)newSpeed;
}

void LEDController::nextColorPreset() {
  const uint32_t presets[] = {
    Color(255, 0, 0),    // Red
    Color(255, 165, 0),  // Orange
    Color(255, 255, 0),  // Yellow
    Color(144, 238, 144),// Light Green
    Color(0, 255, 0),    // Green
    Color(0, 255, 255),  // Cyan
    Color(0, 0, 255),    // Blue
    Color(0, 0, 139),    // Dark Blue
    Color(255, 0, 255),  // Magenta
    Color(255, 192, 203) // Pink
  };
  const uint8_t count = sizeof(presets) / sizeof(presets[0]);
  
  colorPresetIndex = (colorPresetIndex + 1) % count;
  setSolidColor(presets[colorPresetIndex]);
}

void LEDController::setRandomAnimation() {
  int randAnim = random(1, ANIM_COUNT);
  setAnimation((AnimationType)randAnim, random(30, 100));
  startAnimation();
}
