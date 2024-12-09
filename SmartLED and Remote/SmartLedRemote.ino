#include <ArduinoBLE.h>

#define SERVICE_UUID "180A"
#define CHARACTERISTIC_UUID "2A57"

//버튼 핀 번호 정의
#define BUTTON_ON 2
#define BUTTON_OFF 3
#define BUTTON_AUTO 4
#define BUTTON_UP 5
#define BUTTON_DOWN 6

// 버튼 상태
int previousOnState;
int previousOffState;
int previousAutoState;
int previousUpState;
int previousDownState;

//밝기 상태
int currentBrightness = 9;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("BLE initialise failed");
    while (1);
  }

  pinMode(BUTTON_ON, INPUT_PULLUP);
  pinMode(BUTTON_OFF, INPUT_PULLUP);
  pinMode(BUTTON_AUTO, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  BLE.scanForUuid(SERVICE_UUID);
}

void loop() {
  BLEDevice peripheral = BLE.available();
  
  if(peripheral){
    Serial.print(peripheral.localName());
    Serial.print(" ");
    Serial.print(peripheral.address());
    Serial.print(" ");
    Serial.println(peripheral.advertisedServiceUuid());

    if(peripheral.localName() == "202111448_김가을_SmartLED"){
	    return;
		}
    BLE.stopScan();
		controlLED(peripheral);

    BLE.scanForUuid(SERVICE_UUID);
  }
}

void controlLED(BLEDevice peripheral){
	Serial.println("Connecting ... ");
	//기기에 연결
	if(peripheral.connect()){
	  Serial.println("Connected");
	}
	else{
		Serial.println("Failed");
		BLE.scan();
		return;
  }
  
  //서비스가 있는지 검사
  if(peripheral.discoverAttributes()){
	   Serial.println("Attributes discovered");
  }
  else{
	  Serial.println("Attributes discovery Failed");
	  peripheral.disconnect();
    return;
  }
  
  BLECharacteristic ledCharacteristic = peripheral.characteristic("2A57");
    
	if (!ledCharacteristic) {
		Serial.println("Peripheral does not have LED characteristic!");
		peripheral.disconnect();			
		return;
	} 
	else if (!ledCharacteristic.canWrite()) {
		Serial.println("Peripheral does not have a writable LED characteristic!");
		peripheral.disconnect();
		return;
	}
		
	while (peripheral.connected()) {
		handleCommand(ledCharacteristic);
	}
	Serial.println("Peripheral disconnected");
}

void handleCommand(BLECharacteristic& ledCharacteristic) {
  // ON 버튼
  int onState = digitalRead(BUTTON_ON);
  if (onState == LOW && previousOnState == HIGH) {
    sendCommand(ledCharacteristic, 1, "ON");
  }
  previousOnState = onState;

  // OFF 버튼
  int offState = digitalRead(BUTTON_OFF);
  if (offState == LOW && previousOffState == HIGH) {
    sendCommand(ledCharacteristic, 0, "OFF");
  }
  previousOffState = offState;

  // AUTO 버튼
  int autoState = digitalRead(BUTTON_AUTO);
  if (autoState == LOW && previousAutoState == HIGH) {
    sendCommand(ledCharacteristic, 2, "AUTO");
  }
  previousAutoState = autoState;

  // UP 버튼
  int upState = digitalRead(BUTTON_UP);
  if (upState == LOW && previousUpState == HIGH) {
    if (currentBrightness < 9) {
      currentBrightness += 1;
    }
    int command = currentBrightness + 10; 
    sendCommand(ledCharacteristic, command, "UP");
  }
  previousUpState = upState;

  // DOWN 버튼
  int downState = digitalRead(BUTTON_DOWN);
  if (downState == LOW && previousDownState == HIGH) {
    if (currentBrightness > 0) {
      currentBrightness -= 1;
    }
    int command = currentBrightness + 10; 
    sendCommand(ledCharacteristic, command, "DOWN");
  }
  previousDownState = downState;
}
	
void sendCommand(BLECharacteristic& ledCharacteristic, int command, const char* buttonName) {
  Serial.print("Command Sending: ");
  Serial.println(command);
  ledCharacteristic.writeValue((byte)command);
}