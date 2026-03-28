#ifndef IR_CONTROLLER_H
#define IR_CONTROLLER_H

#include <cstdint>

// IR Sensor Configuration
#define IR_RECEIVE_PIN 2  // Must support interrupts (D2 or D3 on Arduino R4)
#define IR_FEEDBACK_LED 13 // Optional: LED to show IR received

// IR Remote Button Codes (Color remote with brightness control)
// Address: 0xFBE2 (all buttons)
// Format: Command value from NEC protocol
enum IRButton {
  // Power controls
  IR_OFF         = 0x25,
  IR_ON          = 0x26,
  IR_AUTO        = 0x2D,
  
  // Mode/Animation controls
  IR_MODE_PLUS   = 0x0B,
  IR_MODE_MINUS  = 0x0A,
  
  // Brightness/Speed controls
  IR_BRIGHTNESS_UP   = 0x0F,
  IR_BRIGHTNESS_DOWN = 0x6E,
  IR_SPEED_PLUS      = 0x5B,
  IR_SPEED_MINUS     = 0x5C,
  
  // Primary colors
  IR_RED     = 0x06,
  IR_GREEN   = 0x08,
  IR_BLUE    = 0x5A,
  IR_YELLOW  = 0x09,
  
  // Secondary colors
  IR_CYAN    = 0x8A,
  IR_PURPLE  = 0x10,
  IR_PINK    = 0x1C,
  IR_ORANGE  = 0x0C,
  
  // Dark/saturated colors
  IR_DARK_BLUE   = 0x52,
  IR_DARK_GREEN  = 0x53,
  
  // White and special
  IR_WHITE   = 0x1A,
  IR_COLOR_CYCLE = 0x1F,
  
  // Music/function buttons
  IR_MUSIC_1 = 0x01,
  IR_MUSIC_2 = 0x02,
  IR_LOCK    = 0x00,
  
  IR_INVALID = 0xFFFFFFFF
};

// Color button mappings for convenience
enum ColorButton {
  COLOR_RED      = IR_RED,
  COLOR_GREEN    = IR_GREEN,
  COLOR_BLUE     = IR_BLUE,
  COLOR_YELLOW   = IR_YELLOW,
  COLOR_CYAN     = IR_CYAN,
  COLOR_MAGENTA  = IR_PURPLE,
  COLOR_WHITE    = IR_WHITE,
  COLOR_OFF      = IR_OFF
};

// Animation button mappings
enum AnimationButton {
  ANIM_SOLID_BTN   = IR_WHITE,
  ANIM_RAINBOW_BTN = IR_COLOR_CYCLE,
  ANIM_PULSE_BTN   = IR_MODE_PLUS,
  ANIM_CHASE_BTN   = IR_MODE_MINUS,
  ANIM_BLINK_BTN   = IR_MUSIC_1
};

class IRController {
public:
  IRController();
  ~IRController();
  
  void begin();
  void update();
  
  // Query methods
  bool hasCommand();
  uint32_t getLastCommand() const;
  uint32_t getLastCommandTime() const;
  uint8_t getRepeatCount() const;
  
  // Callback support
  typedef void (*IRCallback)(uint32_t command);
  void setCallback(IRCallback callback);
  
  // Debug
  void printCommand(uint32_t command) const;
  
private:
  uint32_t lastCommand;
  uint32_t lastCommandTime;
  uint8_t repeatCount;
  bool newCommandReceived;
  IRCallback userCallback;
  
  void handleCommand(uint32_t command);
};

#endif
