#pragma once
#include <MFRC522.h>
#include "config.h"
#include "result.h"
#include "trace.h"

#define SS_PIN 32
#define RST_PIN 33

class CardReader
{
public:

    struct CardSecret
    {
        byte secret[32];
    };
    struct Uid {
		byte		size;
		byte		uidByte[10];
		byte		sak;			
	};

    CardReader();
    Result begin();
    Result read(CardReader::Uid& iUid, CardReader::CardSecret& iSecret);

private:
    Result wakeupAndSelect(CardReader::Uid& iUid);
    Result authWithSecretKey();
    Result readCardSecret(CardReader::CardSecret& iSecret);
    void endCard();

private:
    MFRC522 mMFRC;

    byte mSecretKey[16] = PICC_PSK;
};



inline CardReader::CardReader()
: mMFRC(SS_PIN, RST_PIN)
{

}

inline Result CardReader::begin()
{
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(RST_PIN, HIGH);

    mMFRC.PCD_Init(SS_PIN, RST_PIN);

    byte version = mMFRC.PCD_ReadRegister(MFRC522::VersionReg);
    if((version == 0x00) || (version == 0xFF))
    {
        X_ERROR("MFRC522 Communication failure");
        return Result::ERROR;
    }

    mMFRC.PCD_WriteRegister(MFRC522::TxModeReg, 0x00);
    mMFRC.PCD_WriteRegister(MFRC522::RxModeReg, 0x00);

    mMFRC.PCD_WriteRegister(MFRC522::ModWidthReg, 0x26);
    delay(5);

    return Result::OK;
}

inline Result CardReader::read(CardReader::Uid& iUid, CardReader::CardSecret& iSecret)
{
    if(begin() != Result::OK) {
		X_ERROR("Could not initialize RC522!");
		return Result::ERROR;
	}
	if (wakeupAndSelect(iUid) != Result::OK) {
		//X_ERROR("Could not select card!");
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

inline Result CardReader::wakeupAndSelect(CardReader::Uid& iUid)
{

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

    status = mMFRC.PICC_Select(reinterpret_cast<MFRC522::Uid*>(&iUid), 0);

    if (status != MFRC522::STATUS_OK) {
        endCard();
        return Result::ERROR;
    }
    return Result::OK;
}

inline Result CardReader::authWithSecretKey()
{
    MFRC522::StatusCode status;
    status = mMFRC.MIFARE_UL_C_Auth(mSecretKey);
    if (status != MFRC522::STATUS_OK) {
        endCard();
        return Result::ERROR;
    }
	X_INFO("authentication with secret key success");
	return Result::OK;
}

inline Result CardReader::readCardSecret(CardReader::CardSecret& iSecret)
{
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
                X_ERROR("expected answer length 18, got ",readBufferSize);

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

inline void CardReader::endCard()
{
    mMFRC.PCD_StopCrypto1();
    mMFRC.PICC_HaltA();
}
