#include "LEDController.h"
#include "IRController.h"
#include "WiFiController.h"
#include "BLEController.h"
#include "StateManager.h"

LEDController ledController;
IRController irController;
WiFiController wifiController;
BLEController bleController;
StateManager stateManager;

void handleIRCommand(uint32_t command);

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=== RGB Floor Lamp Starting ===");
  Serial.println("[Setup] Serial initialized");
  
  // Initialize LED controller
  Serial.println("[Setup] Initializing LED controller...");
  ledController.begin();
  Serial.println("[Setup] LED controller OK");
  
  // Initialize IR controller
  Serial.println("[Setup] Initializing IR controller...");
  irController.begin();
  Serial.println("[Setup] IR controller OK");
  
  // Initialize WiFi controller
  Serial.println("[Setup] Initializing WiFi controller...");
  wifiController.begin();
  Serial.println("[Setup] WiFi controller OK");
  
  // Initialize BLE controller
  Serial.println("[Setup] Initializing BLE controller...");
  bleController.begin();
  Serial.println("[Setup] BLE controller OK");
  
  // Start with white solid color
  Serial.println("[Setup] Setting white color...");
  ledController.setSolidColor(255, 255, 255);
  
  Serial.println("\n[Setup] Complete! Ready for control.");
  Serial.println("Press IR remote buttons now...");
}

void loop() {
  // Check for IR commands frequently
  irController.update();
  if (irController.hasCommand()) {
    uint32_t command = irController.getLastCommand();
    Serial.println("[Loop] IR command received");
    handleIRCommand(command);
  }
  
  // Yield to allow interrupt handlers to run
  yield();
  
  // Update LED animations
  ledController.update();
  
  // Yield after LED update to allow interrupts
  yield();
  
  // Update WiFi
  wifiController.update();
  
  // Update BLE
  bleController.update();
  
  // Yield at end of loop
  yield();
}

void handleIRCommand(uint32_t command) {
  Serial.print("[Main] IR Command: 0x");
  Serial.println(command, HEX);
  
  // Map command to IR button enum
  // (we'll decode these based on actual values)
  
  // Power control
  if (command == IR_OFF) {
    stateManager.saveLampState(ledController);
    ledController.turnOff();
    Serial.println(">> Lamp OFF (state saved)");
    return;
  }
  
  if (command == IR_ON) {
    ledController.turnOn();
    if (stateManager.hasSavedState()) {
      stateManager.restoreLampState(ledController);
      Serial.println(">> Lamp ON (state restored)");
    } else {
      Serial.println(">> Lamp ON (no saved state)");
    }
    return;
  }
  
  // Only process color/animation commands if lamp is on
  if (!ledController.isOn()) {
    return;
  }
  
  // Color selection
  switch (command) {
    case IR_RED:
      ledController.stopAnimation();
      ledController.setSolidColor(255, 0, 0);
      Serial.println(">> Color: RED");
      break;
    case IR_GREEN:
      ledController.stopAnimation();
      ledController.setSolidColor(0, 255, 0);
      Serial.println(">> Color: GREEN");
      break;
    case IR_BLUE:
      ledController.stopAnimation();
      ledController.setSolidColor(0, 0, 255);
      Serial.println(">> Color: BLUE");
      break;
    case IR_YELLOW:
      ledController.stopAnimation();
      ledController.setSolidColor(255, 255, 0);
      Serial.println(">> Color: YELLOW");
      break;
    case IR_CYAN:
      ledController.stopAnimation();
      ledController.setSolidColor(0, 255, 255);
      Serial.println(">> Color: CYAN");
      break;
    case IR_PURPLE:
      ledController.stopAnimation();
      ledController.setSolidColor(255, 0, 255);
      Serial.println(">> Color: MAGENTA");
      break;
    case IR_PINK:
      ledController.stopAnimation();
      ledController.setSolidColor(255, 105, 180);
      Serial.println(">> Color: PINK");
      break;
    case IR_ORANGE:
      ledController.stopAnimation();
      ledController.setSolidColor(255, 165, 0);
      Serial.println(">> Color: ORANGE");
      break;
    case IR_DARK_BLUE:
      ledController.stopAnimation();
      ledController.setSolidColor(0, 0, 139);
      Serial.println(">> Color: DARK BLUE");
      break;
    case IR_DARK_GREEN:
      ledController.stopAnimation();
      ledController.setSolidColor(0, 100, 0);
      Serial.println(">> Color: DARK GREEN");
      break;
    case IR_WHITE:
      ledController.stopAnimation();
      ledController.setSolidColor(255, 255, 255);
      Serial.println(">> Color: WHITE");
      break;
    
    // Animations
    case IR_COLOR_CYCLE:
      ledController.nextColorPreset();
      Serial.println(">> Color Preset Cycle");
      break;
    case IR_MODE_PLUS:
      ledController.nextAnimation();
      Serial.println(">> Next Animation");
      break;
    case IR_MODE_MINUS:
      ledController.prevAnimation();
      Serial.println(">> Prev Animation");
      break;
    case IR_AUTO:
      ledController.setRandomAnimation();
      Serial.println(">> Random Animation");
      break;
    case IR_LOCK:
      ledController.freezeAnimation();
      Serial.print(">> Animation Freeze: ");
      Serial.println(ledController.isFrozen() ? "ON" : "OFF");
      break;
    case IR_SPEED_PLUS: {
      ledController.adjustSpeed(-10); // Decrease interval = increase speed
      Serial.print(">> Speed Increase. Interval: ");
      Serial.println(ledController.getAnimationSpeed());
      break;
    }
    case IR_SPEED_MINUS: {
      ledController.adjustSpeed(10); // Increase interval = decrease speed
      Serial.print(">> Speed Decrease. Interval: ");
      Serial.println(ledController.getAnimationSpeed());
      break;
    }
    case IR_MUSIC_1:
      // Unassigned
      Serial.println(">> MUSIC 1 (Unassigned)");
      break;
    case IR_MUSIC_2:
      // Unassigned
      Serial.println(">> MUSIC 2 (Unassigned)");
      break;
    
    // Brightness control
    case IR_BRIGHTNESS_UP: {
      uint8_t brightness = ledController.getBrightness();
      if (brightness < 245) {
        brightness += 10;
      } else {
        brightness = 255;
      }
      ledController.setBrightness(brightness);
      Serial.print(">> Brightness: ");
      Serial.println(brightness);
      break;
    }
    case IR_BRIGHTNESS_DOWN: {
      uint8_t brightness = ledController.getBrightness();
      if (brightness > 10) {
        brightness -= 10;
      } else {
        brightness = 5;
      }
      ledController.setBrightness(brightness);
      Serial.print(">> Brightness: ");
      Serial.println(brightness);
      break;
    }
  }
}
