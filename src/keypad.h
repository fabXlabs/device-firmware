#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "trace.h"

#define SPI_MOSI 23
#define SPI_MISO  19
#define SPI_SCK  18
#define SPI_CS 26 // CS (NFC_IRQ)

class Keypad {
public:

  enum class Command : uint8_t {
    NO_COMMAND,     /** < Do nothing, can be used to just poll the state */
    IDLE,           /** < Go to into idle state */
    CODE_REQUIRED,  /** < Card scanned, second factor required => ENTER_CODE */
    ACT_BUSY,       /** < Backend is checking the code => BUSY  */
    AUTH_OK,        /** < Authentication successful */
    AUTH_FAIL_CARD, /** < Authentication failed, card unknown */
    AUTH_FAIL_CODE, /** < Authentication failed, code wrong */
    TOOL_UNLOCKED,  /** < Tool unlocked */
    TOOL_LOCKED,    /** < Tool locked */
    REBOOT,         /** < Reboot microcontroller */
  };

  /**
   * Request struct, send from FabXDevice > Keypad
   */
  struct CommandBuffer {
    uint32_t Spare : 24; /** < Not yet used */
    Keypad::Command Command : 8; /** < Controls the keypad's target state */
  };

  enum class State : uint8_t {
    IDLE,         /** < Idle, show logo screen, wait for FabX to scan card */
    ENTER_CODE,   /** < Card scanned, code as second factor required */
    TYPING,       /** < Informative state, user is typing (in ENTER_CODE) */
    CODE_READY,   /** < Code entered and ready for FabX to verify */
    BUSY,         /** < Display progress indicator */
    SUCCESS,      /** < Authentication successful (code ok or not required) */
    UNLOCKED,     /** < Tool unlocked */
    LOCKED,       /** < Tool locked */
    UNKNOWN_CARD, /** < Authentication failed, card unknown */
    WRONG_CODE,   /** < Authentication failed, code wrong */
  };

  /**
   * Response struct, send from Keypad > FabXDevice
   */
  struct StateBuffer {
    uint32_t Code : 20; /** < Entered code as 20-bit unsigned integer,
                              which is sufficient for up to 6 digit numbers. */
    uint8_t CodeLen : 4; /** < Length of entered code,
                               allows handling codes with leading 0 */
    Keypad::State State : 8; /** < The keypad's actual state */
  };

  void begin();
  void update();
  void reboot();
  Keypad::State getState() const;
  Keypad::Command getCommand() const;
  void setCommand(Keypad::Command cmd);
  const char* getCodeAsString() const;
  uint32_t getRaw() const;
  unsigned long mFailCounter = 0;
  unsigned long mCounter = 0;
private:
  CommandBuffer mCmd{};
  StateBuffer mState{};
  unsigned long mLastCommand{};
};

void Keypad::begin()
{
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  pinMode(SPI_CS,OUTPUT);
  digitalWrite(SPI_CS,HIGH); //disable device
}

void Keypad::update()
{
  if (millis()-mLastCommand > 1000) mCmd.Command = Keypad::Command::NO_COMMAND;
  SPI.beginTransaction(SPISettings(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0));
  digitalWrite(SPI_CS,LOW); //enable device
  SPI.transferBytes(reinterpret_cast<uint8_t*>(&mCmd),
                    reinterpret_cast<uint8_t*>(&mState), 4);
  digitalWrite(SPI_CS,HIGH); //disable device
  SPI.endTransaction();
  if (*(reinterpret_cast<uint32_t*>(&mState)) != 0x12345678) mFailCounter++;
  mCounter++;
  // X_DEBUG("Keypad C:%d S:%d", static_cast<int>(mCmd.Command),
  //                             static_cast<int>(mState.State));
}

void Keypad::reboot()
{
  setCommand(Keypad::Command::REBOOT);
}

Keypad::State Keypad::getState() const
{
  return mState.State;
}

void Keypad::setCommand(Keypad::Command cmd)
{
  mCmd.Command = cmd;
  mLastCommand = millis();
}

Keypad::Command Keypad::getCommand() const
{
  return mCmd.Command;
}

const char* Keypad::getCodeAsString() const
{
  static char buf[8];
  snprintf(buf, sizeof(buf), "%.*lu", mState.CodeLen, mState.Code);
  return buf;
}

uint32_t Keypad::getRaw() const
{
  return *(reinterpret_cast<const uint32_t*>(&mState));
}