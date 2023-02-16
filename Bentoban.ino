/**
*プログラム名：BentoBan
*機能：電源ON/OFFの機能を搭載したブロードキャスト方式のBLEデバイス。一定時間が経過すると自動で電源オフにできる機能もあります！
*日付：2022/07/21
*作者：上之薗和人
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

const int powerPin = 12;
const int buttonPin = 14;
const int ledPin = 16; //電源オン時に光る
const int bluetoothPin = 17; //bluetooth接続時に光る

int button_count = 0;
int power_value = 0;
int button_value = 0;
//int time_count = 0;

boolean flag = true;

const int killtime = 300000;//眠るまでの時間[ms]
unsigned long start_time;


#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BUTTON_PIN_BITMASK 0x14

class MyServerCallbacks: public BLEServerCallbacks{ //bluetoothが繋がっているか否か
  
    void onConnect(BLEServer* pServer){
      
      deviceConnected = true;
      
    };

    void onDisconnect(BLEServer* pServer){
      
      deviceConnected = false;
      
    }
    
};

void ble(){
  
  // デバイス名
  BLEDevice::init("Bentoban");

  //BLEサーバー作成
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  //BLEサービス作成
  BLEService *pService = pServer->createService(SERVICE_UUID);

  //BLEキャラクタリスティック作成
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  //BLEDescriptor作成
  pCharacteristic->addDescriptor(new BLE2902());

  //サービス開始
  pService->start();

  //アドバタイジング開始
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("Waiting a client connection to notify...");
  
}

void bleConnect(){

  start_time = millis();
  
  Serial.println(start_time);
  //Serial.println(time_count);

  // notify changed value
  if(deviceConnected){
    
    digitalWrite(bluetoothPin, HIGH);  
    Serial.println("接続中です。");

    if(!deviceConnected){ //接続が切れたらもう一度接続しに行く
          
        ble();
        
        }
          
  }else{

    digitalWrite(bluetoothPin, LOW);
    Serial.println("接続待ちです。");
        
  }
        
  pCharacteristic->notify();
    
  // disconnecting
  if(!deviceConnected && oldDeviceConnected){
    
    delay(500);
    pServer->startAdvertising();
    Serial.println("アドバタイジング開始");
    oldDeviceConnected = deviceConnected;
    
    }
  
  if(deviceConnected && !oldDeviceConnected){
    
    oldDeviceConnected = deviceConnected;
        
  }
  
}


void powerOnOperation(){ //電源ボタン制御

  if( button_value == HIGH ){

      button_count++;

      if ( button_count >= 150 ) {
        
      Serial.print("電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！電源オン！"); //button_countが150までカウントできているか

      //time_count++;
      
      digitalWrite(ledPin, HIGH);
      digitalWrite(powerPin, HIGH);
        
      button_count = 0;
        
      }
      
    } else {

      button_count = 0;
      
    }
      
}


void powerOffOperation(){ //自動電源オフ

  if(start_time > killtime){
    
    digitalWrite(ledPin, LOW);
    digitalWrite(bluetoothPin, LOW);
    digitalWrite(powerPin, LOW);

    start_time = 0;//スタートタイムをリセット
      
  }
  
}


void setup() {
  
  Serial.begin(115200);
  pinMode(powerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(ledPin, OUTPUT);
  pinMode(bluetoothPin, OUTPUT);  

  ble();

}


void loop() {

  power_value = digitalRead(powerPin);
  button_value = digitalRead(buttonPin);

  Serial.println("power_value = " + String(power_value));
  Serial.println(" button_value = " + String(button_value));
  Serial.println(" button_count = " + String(button_count));

  bleConnect();
  powerOnOperation();
  powerOffOperation();
  delay(10);
  
}
