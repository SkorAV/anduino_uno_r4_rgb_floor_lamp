#include "IRController.h"
#include <Arduino.h>

#ifndef IRREMOTE_INCLUDED
#define IRREMOTE_INCLUDED
#include <IRremote.hpp>
#endif

IRController::IRController()
  : lastCommand(IR_INVALID),
    lastCommandTime(0),
    repeatCount(0),
    newCommandReceived(false),
    userCallback(nullptr) {
}

IRController::~IRController() {
}

void IRController::begin() {
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
#ifdef DEBUG
  Serial.println("[IR] IR Receiver initialized");
  Serial.print("[IR] Pin: ");
  Serial.println(IR_RECEIVE_PIN);
#endif
}

void IRController::update() {
  if (IrReceiver.decode()) {
    // Only accept NEC protocol with address 0xFBE2
    if (IrReceiver.decodedIRData.protocol != NEC || 
        IrReceiver.decodedIRData.address != 0xFBE2) {
      
#ifdef DEBUG
      Serial.print("[IR] IGNORED - Protocol: ");
      Serial.print(IrReceiver.decodedIRData.protocol);
      Serial.print(" Address: 0x");
      Serial.println(IrReceiver.decodedIRData.address, HEX);
#endif
      
      IrReceiver.resume();
      return;  // Ignore other protocols/addresses
    }
    
    uint32_t command = IrReceiver.decodedIRData.command;
    
    // Handle repeat codes (same button held down)
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      repeatCount++;
    } else {
      repeatCount = 0;
      lastCommand = command;
      lastCommandTime = millis();
      newCommandReceived = true;
      
#ifdef DEBUG
      Serial.print("[IR] Command: 0x");
      Serial.println(command, HEX);
#endif
    }
    
    if (userCallback != nullptr) {
      userCallback(command);
    }
    
    IrReceiver.resume();
  }
}

bool IRController::hasCommand() {
  bool result = newCommandReceived;
  newCommandReceived = false;
  return result;
}

uint32_t IRController::getLastCommand() const {
  return lastCommand;
}

uint32_t IRController::getLastCommandTime() const {
  return lastCommandTime;
}

uint8_t IRController::getRepeatCount() const {
  return repeatCount;
}

void IRController::setCallback(IRCallback callback) {
  userCallback = callback;
}

void IRController::printCommand(uint32_t command) const {
  switch (command) {
    // Power controls
    case IR_OFF:
      Serial.println("OFF");
      break;
    case IR_ON:
      Serial.println("ON");
      break;
    case IR_AUTO:
      Serial.println("AUTO");
      break;
    
    // Mode/Animation controls
    case IR_MODE_PLUS:
      Serial.println("MODE+");
      break;
    case IR_MODE_MINUS:
      Serial.println("MODE-");
      break;
    
    // Brightness/Speed
    case IR_BRIGHTNESS_UP:
      Serial.println("BRIGHTNESS+");
      break;
    case IR_BRIGHTNESS_DOWN:
      Serial.println("BRIGHTNESS-");
      break;
    case IR_SPEED_PLUS:
      Serial.println("SPEED+");
      break;
    case IR_SPEED_MINUS:
      Serial.println("SPEED-");
      break;
    
    // Primary colors
    case IR_RED:
      Serial.println("RED");
      break;
    case IR_GREEN:
      Serial.println("GREEN");
      break;
    case IR_BLUE:
      Serial.println("BLUE");
      break;
    case IR_YELLOW:
      Serial.println("YELLOW");
      break;
    
    // Secondary colors
    case IR_CYAN:
      Serial.println("CYAN");
      break;
    case IR_PURPLE:
      Serial.println("PURPLE");
      break;
    case IR_PINK:
      Serial.println("PINK");
      break;
    case IR_ORANGE:
      Serial.println("ORANGE");
      break;
    
    // Dark colors
    case IR_DARK_BLUE:
      Serial.println("DARK BLUE");
      break;
    case IR_DARK_GREEN:
      Serial.println("DARK GREEN");
      break;
    
    // Special
    case IR_WHITE:
      Serial.println("WHITE");
      break;
    case IR_COLOR_CYCLE:
      Serial.println("COLOR CYCLE");
      break;
    case IR_MUSIC_1:
      Serial.println("MUSIC 1");
      break;
    case IR_MUSIC_2:
      Serial.println("MUSIC 2");
      break;
    case IR_LOCK:
      Serial.println("LOCK");
      break;
    
    default:
      Serial.print("UNKNOWN (0x");
      Serial.print(command, HEX);
      Serial.println(")");
      break;
  }
}
