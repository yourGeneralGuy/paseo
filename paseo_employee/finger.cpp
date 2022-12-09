#include <M5StickCPlus.h>
#include "finger.h"

FingerPrint::FingerPrint(void) {
}

FingerPrint FP; // Creates a fingerprint object


// Custom function to get fingerprint data; modification of sendAndRecieve function
//  Returns: An acknowledgement of successful fingerprint read (uint8_t) 
uint8_t FingerPrint::fpm_bufferCollect(uint16_t timeout) {
    uint8_t i, j;
    uint8_t checkSum = 0;

    FP.RxCnt    = 0;
    FP.TxBuf[5] = 0;

    Serial2.write(CMD_HEAD);
    for (i = 1; i < 6; i++) {
        Serial2.write(FP.TxBuf[i]);
        checkSum ^= FP.TxBuf[i];  // Sent the element at position i to zero because -> checkSum = checkSum ^ FP.TxBuf[i]
    }
    Serial2.write(checkSum);
    Serial2.write(CMD_TAIL);

    // This while loop is meant to wait for a response from the fingerprint scanner
    while (FP.RxCnt < 8 && timeout > 0) { // While the count of elements in the reciever buffer is < 8 and passed
                                          // timeout parameter is greater than 0...
        delay(1);   // Delay operation for a second
        timeout--;  // Decrement 'timeout' by 1
    }

    uint8_t ch; // Create a variable that, 'Channel' or 'ch', that write int(s) into receiver buffer
    for (i = 0; i < 8; i++) {     // For the length of an expected message...
        if (Serial2.available()) {  // Check availabilty of Serial pin 2
            ch = Serial2.read();  // Store what the pin has to say in ch
            FP.RxCnt++;           // Record that the pin said something
            FP.RxBuf[i] = ch;     // Now store ch in as an element in the Reciever buffer
        }
    }

    if (FP.RxCnt != 8) {  // If you don't not get a complete returned message...
        FP.RxCnt = 0;     // reset the count
        return ACK_TIMEOUT; // Acknowledge the failed 
    }
    if (FP.RxBuf[HEAD] != CMD_HEAD) return ACK_FAIL;        // If you don't find 245 at position 0 of buffer -> acknowledge failure
    if (FP.RxBuf[TAIL] != CMD_TAIL) return ACK_FAIL;        // If you don't find 245 at position 7 of the buffer -> acknowledge failure
    if (FP.RxBuf[CMD] != (FP.TxBuf[CMD])) return ACK_FAIL;  // If the recieve command is not the same as the transmitted command 
                                                            // -> acknoweldge failed send

    buffCollect = FP.RxBuf[9];
    // Need to include variable to return RxBuff before it is wiped.
    checkSum = 0; // 
    for (j = 1; j < CHK; j++) { // For j, =1, < CHK, 6...
        checkSum ^= FP.RxBuf[j];  // reset the Receiving buffer
    }
    if (checkSum != FP.RxBuf[CHK]) { // If your check buffer is != 0
        return ACK_FAIL;            // Acknowledge an overrun
    }
    // M5.Lcd.print(FP.RxBuf[9]); // TEST CODE!!!!! FP.RxBuf
    // return buffCollect;
    return ACK_SUCCESS;
}

uint8_t FingerPrint::fpm_sendAndReceive(uint16_t timeout) {
    uint8_t i, j;
    uint8_t checkSum = 0;

    FP.RxCnt    = 0;
    FP.TxBuf[5] = 0;

    Serial2.write(CMD_HEAD);
    for (i = 1; i < 6; i++) {
        Serial2.write(FP.TxBuf[i]);
        checkSum ^= FP.TxBuf[i];  // Sent the element at position i to zero because -> checkSum = checkSum ^ FP.TxBuf[i]
    }
    Serial2.write(checkSum);
    Serial2.write(CMD_TAIL);

    // This while loop is meant to wait for a response from the fingerprint scanner
    while (FP.RxCnt < 8 && timeout > 0) { // While the count of elements in the reciever buffer is < 8 and passed
                                          // timeout parameter is greater than 0...
        delay(1);   // Delay operation for a second
        timeout--;  // Decrement 'timeout' by 1
    }

    uint8_t ch; // Create a variable that, 'Channel' or 'ch', that write int(s) into receiver buffer
    for (i = 0; i < 8; i++) {     // For the length of an expected message...
        if (Serial2.available()) {  // Check availabilty of Serial pin 2
            ch = Serial2.read();  // Store what the pin has to say in ch
            FP.RxCnt++;           // Record that the pin said something
            FP.RxBuf[i] = ch;     // Now store ch in as an element in the Reciever buffer
        }
    }

    if (FP.RxCnt != 8) {  // If you don't not get a complete returned message...
        FP.RxCnt = 0;     // reset the count
        return ACK_TIMEOUT; // Acknowledge the failed 
    }
    if (FP.RxBuf[HEAD] != CMD_HEAD) return ACK_FAIL;        // If you don't find 245 at position 0 of buffer -> acknowledge failure
    if (FP.RxBuf[TAIL] != CMD_TAIL) return ACK_FAIL;        // If you don't find 245 at position 7 of the buffer -> acknowledge failure
    if (FP.RxBuf[CMD] != (FP.TxBuf[CMD])) return ACK_FAIL;  // If the recieve command is not the same as the transmitted command 
                                                            // -> acknoweldge failed send

    // Need to include variable to return RxBuff before it is wiped.
    checkSum = 0; // 
    for (j = 1; j < CHK; j++) { // For j, =1, < CHK, 6...
        checkSum ^= FP.RxBuf[j];  // reset the Receiving buffer
    }
    if (checkSum != FP.RxBuf[CHK]) { // If your check buffer is != 0
        return ACK_FAIL;            // Acknowledge an overrun
    }
    // M5.Lcd.print(FP.RxBuf[9]); // TEST CODE!!!!! FP.RxBuf
    return ACK_SUCCESS;
}

uint8_t FingerPrint::fpm_sleep(void) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_SLEEP_MODE;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = 0;
    FP.TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(500);

    if (res == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_setAddMode(uint8_t fpm_mode) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_ADD_MODE;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = fpm_mode;
    FP.TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_readAddMode(void) {
    FP.TxBuf[CMD] = CMD_ADD_MODE;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = 0;
    FP.TxBuf[P3]  = 0X01;

    fpm_sendAndReceive(200);

    return FP.RxBuf[Q2];
}

uint16_t FingerPrint::fpm_getUserNum(void) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_USER_CNT;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = 0;
    FP.TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return FP.RxBuf[Q2];
    } else {
        return 0XFF;
    }
}

uint8_t FingerPrint::fpm_deleteAllUser(void) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_DEL_ALL;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = 0;
    FP.TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_deleteUser(uint8_t userNum) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_DEL;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = userNum;
    FP.TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(200);

    if (res == ACK_SUCCESS && RxBuf[Q3] == ACK_SUCCESS) {
        return ACK_SUCCESS;
    } else {
        return ACK_FAIL;
    }
}

uint8_t FingerPrint::fpm_addUser(uint8_t userNum, uint8_t userPermission) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_ADD_1;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = userNum;
    FP.TxBuf[P3]  = userPermission;

    res = fpm_sendAndReceive(3000);

    if (res == ACK_SUCCESS) {
        if (FP.RxBuf[Q3] == ACK_SUCCESS) {  //If M5Stick, receiving data from scanner, at position Q3 in buffer
                                            //is equal to ACK_SUCCESS...
            FP.TxBuf[CMD] = CMD_ADD_2;  //Update M5Stick, transmitting to scanner, buffer at position CMD 2 hex

            res = fpm_sendAndReceive(3000);

            if (res == ACK_SUCCESS) {
                if (FP.RxBuf[Q3] == ACK_SUCCESS) {
                    FP.TxBuf[CMD] = CMD_ADD_3;
                    res           = fpm_sendAndReceive(3000);
                    if (res == ACK_SUCCESS) {
                        return FP.RxBuf[Q3];
                    }
                }
            }
        }
    }
    return res;
}

uint8_t FingerPrint::fpm_compareFinger(void) {
    uint8_t res;

    FP.TxBuf[CMD] = CMD_MATCH;
    FP.TxBuf[P1]  = 0;
    FP.TxBuf[P2]  = 0;
    FP.TxBuf[P3]  = 0;

    res = fpm_sendAndReceive(3000);

    if (res == ACK_SUCCESS) {
        if (FP.RxBuf[Q3] == ACK_NOUSER) {
            return ACK_NOUSER;
        }
        if (FP.RxBuf[Q3] == ACK_TIMEOUT) {
            return ACK_TIMEOUT;
        }
        if ((FP.RxBuf[Q2] != 0) &&
            (FP.RxBuf[Q3] == 1 || FP.RxBuf[Q3] == 2 || FP.RxBuf[Q3] == 3)) {
            return ACK_SUCCESS;
        }
    }
    return res;
}
