#pragma once
#include "config.h"
#include "result.h"
#include "trace.h"
#include <MFRC522.h>

#define SS_PIN 32
#define RST_PIN 33

class CardReader {
public:
  struct CardSecret {
    CardSecret() = default;
    CardSecret(String iHex);
    CardSecret(const char* iHex);

    void fromHexString(String iHex);
    String toString();

    bool operator<(const CardSecret& other) const;
    bool operator==(const CardSecret& other) const;
    bool operator!=(const CardSecret& other) const;

    byte secret[32] {};
  };

  struct Uid {
    Uid() = default;
    Uid(String iHex);
    Uid(const char* iHex);

    void fromHexString(String iHex);
    String toString();

    bool operator<(const Uid& other) const;
    bool operator==(const Uid& other) const;
    bool operator!=(const Uid& other) const;

    // don't change, Uid is reinterpret_cast'ed to MFRC522::Uid
    byte size = 0;
    byte uidByte[10] {};
    byte sak = 0;
  };

  CardReader();
  Result begin();
  Result read(CardReader::Uid &iUid, CardReader::CardSecret &iSecret);
  Result createCard(CardReader::Uid &iUid, CardReader::CardSecret &iSecret);
  Result clearCard();

private:
  Result wakeupAndSelect(CardReader::Uid &iUid);
  Result authWithSecretKey();
  Result authWithDefaultKey();
  Result writeCardSecret(CardReader::CardSecret &iSecret);
  Result writeSecretKey();
  Result writeDefaultKey();
  Result writeAuth0(byte auth0);
  Result writeZeros();
  Result readCardSecret(CardReader::CardSecret &iSecret);
  void endCard();

private:
  MFRC522 mMFRC;

  byte mSecretKey[16] = PICC_PSK;
  byte mDefaultKey[16] = {
      0x49, 0x45, 0x4D, 0x4B, 0x41, 0x45, 0x52, 0x42,  // K7 ... K0 key 1
      0x21, 0x4E, 0x41, 0x43, 0x55, 0x4F, 0x59, 0x46}; // K7 ... K0 key 2
};

inline CardReader::CardReader() : mMFRC(SS_PIN, RST_PIN) {}

inline Result CardReader::begin() {
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(RST_PIN, HIGH);

  mMFRC.PCD_Init(SS_PIN, RST_PIN);

  byte version = mMFRC.PCD_ReadRegister(MFRC522::VersionReg);
  if ((version == 0x00) || (version == 0xFF)) {
    X_ERROR("MFRC522 Communication failure");
    return Result::ERROR;
  }

  mMFRC.PCD_WriteRegister(MFRC522::TxModeReg, 0x00);
  mMFRC.PCD_WriteRegister(MFRC522::RxModeReg, 0x00);

  mMFRC.PCD_WriteRegister(MFRC522::ModWidthReg, 0x26);
  delay(5);

  return Result::OK;
}

inline Result CardReader::read(CardReader::Uid &iUid,
                               CardReader::CardSecret &iSecret) {
  if (begin() != Result::OK) {
    X_ERROR("Could not initialize RC522!");
    return Result::ERROR;
  }
  if (wakeupAndSelect(iUid) != Result::OK) {
    // X_ERROR("Could not select card!");
    return Result::ERROR;
  }

  if (authWithSecretKey() != Result::OK) {
    X_ERROR("Could not authenticate with secret key!");
    return Result::ERROR;
  }

  if (readCardSecret(iSecret) != Result::OK) {
    X_ERROR("Could not read cardSecret!");
    return Result::ERROR;
  }

  return Result::OK;
}

inline Result CardReader::createCard(CardReader::Uid &iUid,
                                     CardReader::CardSecret &iSecret) {
  if (begin() != Result::OK) {
    X_ERROR("Could not initialize RC522!");
    return Result::ERROR;
  }
  if (wakeupAndSelect(iUid) != Result::OK) {
    // X_ERROR("Could not select card!");
    return Result::ERROR;
  }
  if (authWithDefaultKey() != Result::OK) {
    X_ERROR("Could not authenticate with secret key!");
    return Result::ERROR;
  }
  if (writeCardSecret(iSecret) != Result::OK) {
    X_ERROR("Could not write cardSecret!");
    return Result::ERROR;
  }
  if (writeSecretKey() != Result::OK) {
    X_ERROR("Could not write secretKey!");
    return Result::ERROR;
  }
  if (writeAuth0(0x20) != Result::OK) {
    X_ERROR("Could not write auth!");
    return Result::ERROR;
  }
  return Result::OK;
}

inline Result CardReader::clearCard() {
  CardReader::Uid uid;
  if (begin() != Result::OK) {
    X_ERROR("Could not initialize RC522!");
    return Result::ERROR;
  }
  if (wakeupAndSelect(uid) != Result::OK) {
    X_ERROR("Could not select card!");
    return Result::ERROR;
  }
  if (authWithSecretKey() != Result::OK) {
    X_ERROR("Could not authenticate with secret key!");
    return Result::ERROR;
  }
  writeZeros();
  if (writeDefaultKey() != Result::OK) {
    X_ERROR("Could not write secretKey!");
    return Result::ERROR;
  }
  writeAuth0(0x30);
  X_DEBUG("Card cleared");
  return Result::OK;
}

inline Result CardReader::wakeupAndSelect(CardReader::Uid &iUid) {

  MFRC522::StatusCode status;
  byte waBufferATQA[2];
  byte waBufferSize = 2;

  waBufferATQA[0] = 0x00;
  waBufferATQA[1] = 0x00;

  status = mMFRC.PICC_WakeupA(waBufferATQA, &waBufferSize);

  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }

  X_INFO("ATQA: %x", waBufferATQA);

  if (waBufferATQA[0] != 0x44 || waBufferATQA[1] != 0x00) {
    X_ERROR("Only Ultralight C supported!");
    endCard();
    return Result::ERROR;
  }

  status = mMFRC.PICC_Select(reinterpret_cast<MFRC522::Uid *>(&iUid), 0);

  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }
  return Result::OK;
}

inline Result CardReader::authWithSecretKey() {
  MFRC522::StatusCode status;
  status = mMFRC.MIFARE_UL_C_Auth(mSecretKey);
  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }
  X_INFO("authentication with secret key success");
  return Result::OK;
}

inline Result CardReader::authWithDefaultKey() {
  MFRC522::StatusCode status;
  status = mMFRC.MIFARE_UL_C_Auth(mDefaultKey);
  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }
  X_INFO("authentication with default key success");

  return Result::OK;
}

inline Result CardReader::writeCardSecret(CardReader::CardSecret &iSecret) {
  MFRC522::StatusCode status;
  byte writeBuffer[4];
  byte cardSecret[32];
  memcpy(cardSecret, iSecret.secret, 32);
  X_DEBUG("cardSecret: %s ", iSecret.secret);
  for (int p = 0x20; p <= 0x27; p++) {
    memcpy(writeBuffer, &cardSecret[(p - 0x20) * 4], 4);
    status = mMFRC.MIFARE_Ultralight_Write(p, writeBuffer, 4);
    if (status != MFRC522::STATUS_OK) {

      endCard();
      return Result::ERROR;
    }
  }

  X_INFO("cardSecret written");
  return Result::OK;
}

inline Result CardReader::writeSecretKey() {
  MFRC522::StatusCode status;
  byte secretKey[16] = PICC_PSK;
  status = mMFRC.MIFARE_UL_C_WriteKey(secretKey);
  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }
  return Result::OK;
}

inline Result CardReader::writeDefaultKey() {
  MFRC522::StatusCode status;
  status = mMFRC.MIFARE_UL_C_WriteKey(mDefaultKey);
  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }
  return Result::OK;
}

inline Result CardReader::writeZeros() {
  byte writeBuffer[4];
  MFRC522::StatusCode status;
  memset(writeBuffer, 0, 4);
  for (int p = 0x04; p <= 0x27; p++) {
    status = mMFRC.MIFARE_Ultralight_Write(p, writeBuffer, 4);
    if (status != MFRC522::STATUS_OK) {
      endCard();
      return Result::ERROR;
    }
  }
  if (writeAuth0(0x30) != Result::OK) {
    return Result::ERROR;
  }
  return Result::OK;
}

inline Result CardReader::writeAuth0(byte auth0) {
  byte writeBuffer[4];
  MFRC522::StatusCode status;
  writeBuffer[0] = auth0;
  writeBuffer[1] = 0;
  writeBuffer[2] = 0;
  writeBuffer[3] = 0;
  status = mMFRC.MIFARE_Ultralight_Write(0x2A, writeBuffer, 4);
  if (status != MFRC522::STATUS_OK) {
    endCard();
    return Result::ERROR;
  }
  return Result::OK;
}

inline Result CardReader::readCardSecret(CardReader::CardSecret &iSecret) {
  byte cardSecret[32];
  memset(cardSecret, 0, 32);
  MFRC522::StatusCode status;

  byte readBuffer[32];
  byte readBufferSize = 32;

  for (int p = 0x20; p <= 0x27; p += 4) {
    memset(readBuffer, 0, 32);
    readBufferSize = 32;
    status = mMFRC.MIFARE_Read(p, readBuffer, &readBufferSize);

    if (status == MFRC522::STATUS_OK) {

      if (readBufferSize == 18) {
        memcpy(&cardSecret[(p - 0x20) * 4], readBuffer, 16);
      } else {
        X_ERROR("expected answer length 18, got ", readBufferSize);
      }
    } else {
      X_DEBUG("Read status = ");
      endCard();
      return Result::ERROR;
    }
  }

  memcpy(&(iSecret.secret), cardSecret, 32);
  return Result::OK;
}

inline void CardReader::endCard() {
  mMFRC.PCD_StopCrypto1();
  mMFRC.PICC_HaltA();
}


void hex2byte(String iHex, byte* iBytes, size_t maxlen) {
  for (int i = 0, j = 0; i < iHex.length() && j<maxlen; i += 2, j++) {
    String byteString = iHex.substring(i, i + 2);
    uint8_t byteValue = static_cast<uint8_t>(strtol(byteString.c_str(), 0, 16));
    iBytes[j] = byteValue;
  }
}

String byteToHex(byte* iBytes, size_t iLen) {
  String hexString = "";
  for (int i = 0; i < iLen; i++) {
    // Add leading zero for single-digit hex values
    if (iBytes[i] < 0x10) {
      hexString += "0";
    }
    hexString += String(iBytes[i], HEX);
  }

  hexString.toUpperCase();
  return hexString;
}


CardReader::CardSecret::CardSecret(String iHex) {
  fromHexString(iHex);
}

CardReader::CardSecret::CardSecret(const char* iHex)
: CardSecret(String(iHex))  {
}


void CardReader::CardSecret::fromHexString(String iHex) {
  hex2byte(iHex, secret, sizeof(secret)/sizeof(secret[0]));
}
String CardReader::CardSecret::toString() {
  return byteToHex(secret, sizeof(secret)/sizeof(secret[0]));
}

bool CardReader::CardSecret::operator<(const CardReader::CardSecret& other) const {
    return memcmp(secret, other.secret, sizeof(secret)/sizeof(secret[0])) < 0;
}

bool CardReader::CardSecret::operator==(const CardReader::CardSecret& other) const {
    return memcmp(secret, other.secret, sizeof(secret)/sizeof(secret[0])) == 0;
}

bool CardReader::CardSecret::operator!=(const CardReader::CardSecret& other) const {
    return !(*this == other);
}



CardReader::Uid::Uid(String iHex) {
  fromHexString(iHex);
  size = (iHex.length() + 1) / 2;
  const byte maxsize = sizeof(uidByte)/sizeof(uidByte[0]);
  if (size > maxsize) size = maxsize;
}

CardReader::Uid::Uid(const char* iHex)
: Uid(String(iHex)) {
}

void CardReader::Uid::fromHexString(String iHex) {
  hex2byte(iHex, uidByte, sizeof(uidByte)/sizeof(uidByte[0]));
}

String CardReader::Uid::toString() {
  return byteToHex(uidByte, size);
}

bool CardReader::Uid::operator<(const CardReader::Uid& other) const {
    return size == other.size && memcmp(uidByte, other.uidByte, size) < 0;
}

bool CardReader::Uid::operator==(const CardReader::Uid& other) const {
    return size == other.size && memcmp(uidByte, other.uidByte, size) == 0;
}

bool CardReader::Uid::operator!=(const CardReader::Uid& other) const {
    return !(*this == other);
}
