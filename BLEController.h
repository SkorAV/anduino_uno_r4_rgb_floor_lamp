#ifndef BLE_CONTROLLER_H
#define BLE_CONTROLLER_H

#include <ArduinoBLE.h>
#include "LEDController.h"
#include "StateManager.h"

// BLE Service and Characteristic UUIDs (Custom)
#define BLE_SERVICE_UUID           "19B10000-E8F2-537E-4F6C-D104768A1214"
#define BLE_CHAR_POWER_UUID        "19B10001-E8F2-537E-4F6C-D104768A1214"
#define BLE_CHAR_COLOR_UUID        "19B10002-E8F2-537E-4F6C-D104768A1214"
#define BLE_CHAR_BRIGHTNESS_UUID   "19B10003-E8F2-537E-4F6C-D104768A1214"
#define BLE_CHAR_ANIMATION_UUID    "19B10004-E8F2-537E-4F6C-D104768A1214"
#define BLE_CHAR_SPEED_UUID        "19B10005-E8F2-537E-4F6C-D104768A1214"

class BLEController {
public:
  BLEController();
  ~BLEController();
  
  void begin();
  void update();
  
  // Update BLE characteristic values from internal state
  void updateCharacteristics();
  
private:
  BLEService ledService;
  BLEByteCharacteristic powerChar;
  BLECharacteristic colorChar;
  BLEByteCharacteristic brightnessChar;
  BLEByteCharacteristic animationChar;
  BLEUnsignedShortCharacteristic speedChar;
  
  void handlePowerChange();
  void handleColorChange();
  void handleBrightnessChange();
  void handleAnimationChange();
  void handleSpeedChange();
};

#endif
