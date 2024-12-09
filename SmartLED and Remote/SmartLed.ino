#include <ArduinoBLE.h>

#define SERVICE_UUID "180A"
#define CHARACTERISTIC_UUID "2A57"

#define LED_PIN 2
#define LDR_PIN A1

//BLE 관련 전역 변수
BLEService ledService(SERVICE_UUID);
BLEByteCharacteristic switchCharacteristic(CHARACTERISTIC_UUID, BLERead | BLEWrite);

//상태 변수
int currentBrightness = 0;
bool autoMode = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  if (!BLE.begin()) {
    Serial.println("BLE 초기화 실패");
    while (1);
  }

  //BLE 서비스 및 특성 추가
  BLE.setLocalName("202111448_김가을_LED");
  BLE.setAdvertisedService(ledService);
  ledService.addCharacteristic(switchCharacteristic);
  BLE.addService(ledService);
  
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();
  
  if(central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      //BLE 명령 수신 감지
      if (switchCharacteristic.written()) {
        int command = switchCharacteristic.value(); // 명령 값 읽기
        
        Serial.print("Received Command : ");
        Serial.println(command);
        
        handleCommand(command);
      }

      //AUTO 모드일 때 조도 센서를 이용하여 LED 제어
      if (autoMode) {
        AutoLED();
      }
    }
  }
}

void handleCommand(int command) {
  if (command == 0) { 
    turnOffLED();
    autoMode = false; // AUTO 모드 비활성화
  } 
  else if (command == 1) { 
    turnOnLED();
    autoMode = false; // AUTO 모드 비활성화
  } 
  else if (command == 2) { 
    autoMode = true; // AUTO 모드 활성화
  } 
  else if (command >= 10 && command <= 19) { 
    currentBrightness = command - 10; // 명령에 따라 밝기 값 설정
    analogWrite(LED_PIN, brightnessToPWM(currentBrightness));
    
    Serial.print("밝기 조정됨: ");
    Serial.println(currentBrightness);
  }
}

//LED ON
void turnOnLED() {
  analogWrite(LED_PIN, brightnessToPWM(9)); // LED 최대 밝기
  Serial.println("LED 켜짐");
}

//LED OFF
void turnOffLED() {
  analogWrite(LED_PIN, 0); // LED 끄기
  Serial.println("LED 꺼짐");
}

void AutoLED() {
  int lightValue = analogRead(LDR_PIN);
  
  //조도 값에 따라 LED 제어
  if (lightValue > 450) { 
    analogWrite(LED_PIN, 255); // 어두울 때 LED 최대 밝기
  } 
  else{ 
    analogWrite(LED_PIN, 0); // 밝을 때 LED 끄기
  }
}

/* 밝기 값(0~9)을 PWM 값(0~255)으로 변환하는 함수
 * @param brightness 0 ~ 9 사이의 밝기 값
 * @return PWM 값 (0 ~ 255)
 */
int brightnessToPWM(int brightness) {
  return map(brightness, 0, 9, 0, 255);
}