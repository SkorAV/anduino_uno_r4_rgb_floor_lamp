#include "BLEController.h"
#include <Arduino.h>

extern LEDController ledController;
extern StateManager stateManager;

BLEController::BLEController()
  : ledService(BLE_SERVICE_UUID),
    powerChar(BLE_CHAR_POWER_UUID, BLERead | BLEWrite | BLENotify),
    colorChar(BLE_CHAR_COLOR_UUID, BLERead | BLEWrite | BLENotify, 3), // R, G, B
    brightnessChar(BLE_CHAR_BRIGHTNESS_UUID, BLERead | BLEWrite | BLENotify),
    animationChar(BLE_CHAR_ANIMATION_UUID, BLERead | BLEWrite | BLENotify),
    speedChar(BLE_CHAR_SPEED_UUID, BLERead | BLEWrite | BLENotify) {
}

BLEController::~BLEController() {
}

void BLEController::begin() {
  if (!BLE.begin()) {
    Serial.println("[BLE] Starting BLE failed!");
    return;
  }

  BLE.setLocalName("LUMINA_Lamp");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(powerChar);
  ledService.addCharacteristic(colorChar);
  ledService.addCharacteristic(brightnessChar);
  ledService.addCharacteristic(animationChar);
  ledService.addCharacteristic(speedChar);

  BLE.addService(ledService);

  // Initial values
  updateCharacteristics();

  BLE.advertise();
  Serial.println("[BLE] BLE Controller Ready - LUMINA_Lamp");
}

void BLEController::update() {
  BLEDevice central = BLE.central();

  if (central) {
#ifdef DEBUG
    Serial.print("[BLE] Connected to central: ");
    Serial.println(central.address());
#endif

    while (central.connected()) {
      if (powerChar.written()) handlePowerChange();
      if (colorChar.written()) handleColorChange();
      if (brightnessChar.written()) handleBrightnessChange();
      if (animationChar.written()) handleAnimationChange();
      if (speedChar.written()) handleSpeedChange();
      
      // Update characteristics if external changes happen (IR/Web)
      // This is a simple way, though it could be optimized with flags
      static uint32_t lastUpdate = 0;
      if (millis() - lastUpdate > 1000) {
        updateCharacteristics();
        lastUpdate = millis();
      }
      
      yield();
    }

#ifdef DEBUG
    Serial.println("[BLE] Central disconnected");
#endif
  }
}

void BLEController::updateCharacteristics() {
  powerChar.writeValue(ledController.isOn() ? 1 : 0);
  
  uint32_t color = ledController.getCurrentColor();
  uint8_t rgb[3];
  rgb[0] = (color >> 16) & 0xFF;
  rgb[1] = (color >> 8) & 0xFF;
  rgb[2] = color & 0xFF;
  colorChar.writeValue(rgb, 3);
  
  brightnessChar.writeValue(ledController.getBrightness());
  animationChar.writeValue((uint8_t)ledController.getCurrentAnimation());
  speedChar.writeValue(ledController.getAnimationSpeed());
}

void BLEController::handlePowerChange() {
  bool power = powerChar.value();
  if (power) {
    ledController.turnOn();
    if (stateManager.hasSavedState()) {
      stateManager.restoreLampState(ledController);
    }
  } else {
    stateManager.saveLampState(ledController);
    ledController.turnOff();
  }
#ifdef DEBUG
  Serial.print("[BLE] Power set to: ");
  Serial.println(power);
#endif
}

void BLEController::handleColorChange() {
  const uint8_t* rgb = colorChar.value();
  ledController.stopAnimation();
  ledController.setSolidColor(rgb[0], rgb[1], rgb[2]);
#ifdef DEBUG
  Serial.print("[BLE] Color set to: R=");
  Serial.print(rgb[0]);
  Serial.print(" G=");
  Serial.print(rgb[1]);
  Serial.print(" B=");
  Serial.println(rgb[2]);
#endif
}

void BLEController::handleBrightnessChange() {
  uint8_t brightness = brightnessChar.value();
  ledController.setBrightness(brightness);
#ifdef DEBUG
  Serial.print("[BLE] Brightness set to: ");
  Serial.println(brightness);
#endif
}

void BLEController::handleAnimationChange() {
  uint8_t animId = animationChar.value();
  if (animId < ANIM_COUNT) {
    ledController.setAnimation((AnimationType)animId, ledController.getAnimationSpeed());
    ledController.startAnimation();
#ifdef DEBUG
    Serial.print("[BLE] Animation set to: ");
    Serial.println(animId);
#endif
  }
}

void BLEController::handleSpeedChange() {
  uint16_t speed = speedChar.value();
  if (speed < 10) speed = 10;
  if (speed > 1000) speed = 1000;
  ledController.setAnimation(ledController.getCurrentAnimation(), speed);
#ifdef DEBUG
  Serial.print("[BLE] Speed set to: ");
  Serial.println(speed);
#endif
}
