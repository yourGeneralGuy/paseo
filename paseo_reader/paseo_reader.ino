#include <M5StickCPlus.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "paseo_reader.h"

//

String strBuff = " Success!";
int status = 2;
int status2;
int timeout = 20000;
int response = 1;

void LcdClear() {
  M5.Lcd.fillScreen(BLACK); // Wipe the screen and set it to BLACK
  M5.Lcd.setCursor(0, 0);   // Set the cursor position so the device may be held vertically
  M5.Lcd.setTextColor(WHITE);    // Set the text color to WHITE for greatest contrast
  M5.Lcd.setTextSize(2);    // Set the text size to 2 for improved visibility
  M5.Lcd.setTextWrap(true); // Enable the text to wrap to prevent broken words
}

void BLEClientSetup() {
  BLEDevice::init("Employee Client.");
  // Notify user of connecting to Bluetooth
  LcdClear();
  M5.Lcd.print("Attempting connection");  
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);    
}

uint8_t BLEclient() {
  M5.Lcd.println("Scanning.");
  if (doConnect == true) {
    if (connectToServer()) {
      M5.Lcd.print("Found Employee.");
    } else {
      M5.Lcd.print("Failed to connect. Try again.");
    }
    doConnect = false;
  }
  if (connected) {
    // Capture the value sent over bluetooth...
    std::string process = pRemoteCharacteristic->readValue();
    String fp = process.c_str();  // Convert the value into a readable string
    M5.Lcd.print(fp); // display for demo purposes! :D
    delay(1000);
    // DEMO: To place a function here that would compare the employee's fingerprint to a record of stored fingerprints
    //        returns strBuff == "Unlocked" (0) if valid employee fingerprint otherwise strBuff == "Locked" if valid employee fingerprint
    //      Not possible to do unless, in this project, as it would require consistent fingerprint placement and external storage to store
    //      fingerprint data for this reader. Therefore, "Success" is returned to show that a BLE connection can be made and that the fingerprint
    //      data can be sent.
    response = 0;
    M5.Lcd.print("Connection made!" + strBuff);
    // strBuff.c_str(), strBuff.length()
    pRemoteCharacteristic->writeValue(response);  // Whatever is written will transform into a string
    delay(100);
    return 1;
  } else if(doScan){
    LcdClear();
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    M5.Lcd.print("Re-scan...");
    return 4;
  }
}


int BLEServerSetup() {
  M5.Lcd.println("Resp. Ret.");

  BLEDevice::init("Hacienda Response");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(serviceUUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         charUUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue(response);
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  return 0;
}

void setup() {
  M5.begin(115200);
  LcdClear();
  M5.Lcd.println("Hacienda Company Card Reader");
}

void loop() {
  if (status == 2) {
    BLEClientSetup();    
  }
  LcdClear();
  if (status != 1) {
    status = BLEclient(); // return status == 1 if connection and status == -1 if failed to connect
  } else if (status == 1) { // if the reader was able to get information from an employee...
    status = BLEServerSetup(); // return status == 0 after initialization
  } else if (status == 0) {
    M5.Lcd.print(".");
    timeout--;
    LcdClear();
    if (timeout == 0) {
      M5.Lcd.print("Timeout.");
      delay(1000);
      LcdClear();
      status = 2; // Restarts the process to search for another employee
    }
  }
}
