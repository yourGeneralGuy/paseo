#include <M5StickCPlus.h>
#include <WiFi.h>
#include "paseo_employee.h"
#include "finger.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <cstdlib>

uint8_t userNum;
int id;
FingerPrint FP_M;
BLECharacteristic* employee;


//Service id

// Imported parameters

  // WiFi connections
const char* ssid = network;
const char* password = pw;
  // Qubitro device parameters
const char* dID = deviceID;
const char* dToken = deviceToken;

// Functions 

  // Screen orientated functions

// Clears the LCD screen so new text may be displayed
void LcdClear() {
  M5.Lcd.fillScreen(BLACK); // Wipe the screen and set it to BLACK
  M5.Lcd.setCursor(0, 0);   // Set the cursor position so the device may be held vertically
  M5.Lcd.setTextColor(WHITE);    // Set the text color to WHITE for greatest contrast
  M5.Lcd.setTextSize(2);    // Set the text size to 2 for improved visibility
  M5.Lcd.setTextWrap(true); // Enable the text to wrap to prevent broken words
}

// ASCII text to display on the LCD screen
void Arrow() {
  M5.Lcd.setCursor(50, 0);
  M5.Lcd.print("^");
  M5.Lcd.setCursor(50, 10);
  M5.Lcd.print("|");
  M5.Lcd.setCursor(0, 0);  
}

  // Key functions to manage and execute prior functions

// Records the user's fingerprint
uint8_t Place() {
  uint8_t match;
  
  // Screen message and formatting
  LcdClear();
  Arrow();
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.println("Place your thumbprint.");

  // Attempt to record user's fingerprint and verify if valid scan
  match = FP_M.fpm_addUser(userNum, 1);
  M5.Lcd.println("User add: " + String(match));
  if (match == ACK_SUCCESS) {
    M5.Lcd.println("Successful Read.");
    return 1;
  } else if (match == ACK_FAIL) {
    M5.Lcd.println("Failed Read.");
    return 0;
  } else if (match == ACK_FULL) { // Might not need.
    M5.Lcd.println("Full");
    return 2;
  } else {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("Timeout.");
    return 2;
  }
}

// Send user fingerprint over bluetooth
void Proceed(uint8_t status) {

  if (status == 1) {  // Verify that a fingerprint was scanned...
    uint8_t status2;  // Used to verify that stored fingerprint is packaged for sending
      M5.Lcd.println("Wait...");  // Give update to user
      status2 = FP_M.fpm_bufferCollect(2000); // Package fingerprint for sending -- return verification
      if (status2 == ACK_FAIL) {  // Check if failure to get fingerprint
        M5.Lcd.println("Error. Try Again.");  // Record if fingerprint fails
      } else {  // Otherwise, pass fingerprint to start process
        BLEProcess(FP_M.buffCollect); // Pass information to reader
      }
    } else if (status == 0) { // If failure
      M5.Lcd.println("Failed Read.");
    } else if (status == -1) {
      M5.Lcd.setTextColor(RED);
      M5.Lcd.print("Timeout.");
    }  
}

  // Bluetooth orientated functions

// Manages the process of connecting to the reader and updating employee on status
void BLEProcess(uint8_t employeeID) {
  uint8_t collect;
  uint8_t timeout = 20000;
  employee = BLEserver(employeeID);
  while (timeout > 0) {
    CheckEmployee(employee);
    --timeout;
    if (timeout == 2000) {
      M5.Lcd.println("Timeout Imminent.");
    }
  }
  // collect = BLEclient();
}

BLECharacteristic* BLEserver(int employeeID) {
  id = employeeID;
  M5.Lcd.println("Starting BLE work!");

  BLEDevice::init("Employee Outbound");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(serviceUUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         charUUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  
  pCharacteristic->setValue(id);
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  return pCharacteristic;
}

void CheckEmployee(BLECharacteristic* pCharacteristic) {
  std::string carry = pCharacteristic->getValue();
  String comp = carry.c_str();
  if (comp == "0") {
    M5.Lcd.print("Unlocked. Proceed!");
    delay(10000);
  } else if (comp == "1") {
    M5.Lcd.print("Locked. Not authorized.");
    delay(10000);
  } else {
    M5.Lcd.print("Not valid code.");
    delay(2000);
  }
  LcdClear();
}

// Setup and M5 operation

void setup() {
  M5.begin();                               // Initializes the hardware on M5StickC+
  Serial2.begin(19200, SERIAL_8N1, 26, 0);  // Open serial ports to communicate with fingerprint scanner
  LcdClear();                               // Clear the LCD screen

  // Show opening text  
  M5.Lcd.setCursor(20, 0); 
  M5.Lcd.print("~Paseo~");
  delay(5000);

  LcdClear(); // Clear the LCD screen
  M5.Lcd.println("Press the  'M5' Buttonto continue");  //Prompt user to scan fingerprint
  userNum = FP_M.fpm_getUserNum();  // Create a number for the user to send information
}

// This program has two functions:
//  Button A (M5)
    // "Place" & "Proceed", Scan a fingerprint and connect over Bluetooth to card reader
//  Button B (Side Button)
    // Demonstration for card reader fingerprint comparison
void loop() {
  uint8_t proceed;
  M5.update();
  if (M5.BtnA.wasPressed()) {
    LcdClear();
    proceed = Place(); // Returns proceed == 1 if successful read, proceed == 0 if failed read, & proceed == -1 if timeout
    if (proceed == 2) {
      Proceed(proceed);
    }
  }
  if (M5.BtnB.wasPressed()) {
    LcdClear();
    M5.Lcd.print('Hurray!');
  }  
}
