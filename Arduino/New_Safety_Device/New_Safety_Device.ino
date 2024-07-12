#include <ESP8266WiFi.h>
#include <espnow.h>

//Replace with ESP receiver broadcoast address.
//MAC address of the head of the rack esp8266
// uint8_t broadcastAddress[] = {0x48, 0x55, 0x19, 0xE4, 0xAF, 0xF2};
//MAC address of the end of the rack esp8266
// uint8_t broadcastAddress[] = {0x48, 0x55, 0x19, 0xE4, 0x4F, 0xD1};

const int led = 5; //LED pin GPIO 5.
const int button = 4; //Push Button connect to pin.
const int signal = 0;
int temp = 0;
//Initial variable to be sent
int status_light = 0;
// //Initial variable to be receieved
int status_rec = 0;

typedef struct struct_message{
  int status;
}struct_message;

// //Created struct_message to store button signal
struct_message readButtonSig;

//Created struct_message to store receiving signal
struct_message incomingButtonSig; 

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingButtonSig, incomingData, sizeof(incomingButtonSig));
  Serial.print("Bytes received: ");
  Serial.println(len);
  status_rec = incomingButtonSig.status;
}

void setup() {

  Serial.begin(115200);

  pinMode(led, OUTPUT); //Declare LED as output.
  pinMode(button, INPUT); //Declare button as input.
  pinMode(signal, OUTPUT);
  Serial.println("");
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if(esp_now_init() != 0){
    Serial.println("Error initailizing ESP-NOW");
    return;
  }

  //Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  //Get trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  //Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  //Callback function when data is recived
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

  temp = digitalRead(button);
  
  if(temp == HIGH){
    if(status_light == 0){
      status_light = 1;
      digitalWrite(led, HIGH);
      digitalWrite(signal, HIGH);
      delay(500);
    }else{
      status_light = 0;
      digitalWrite(led, LOW);
      digitalWrite(signal, LOW);
      delay(500);
    }

    status_rec = status_light;

    readButtonSig.status = status_light;

    esp_now_send(broadcastAddress, (uint8_t *) &readButtonSig, sizeof(readButtonSig));

  }else{
    if(status_rec == 1){
      status_light = 1;
      digitalWrite(led, HIGH);
      digitalWrite(signal, HIGH);
    }else{
      status_light = 0;
      digitalWrite(led, LOW);
      digitalWrite(signal, LOW);
    }
  }
}
