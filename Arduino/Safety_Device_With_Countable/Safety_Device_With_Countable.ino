#include <ESP8266WiFi.h>
#include <espnow.h>

#define THINGSBOARD_ENABLE_PROGMEM 0

#define THINGSBOARD_ENABLE_PSRAM 0
#define THINGSBOARD_ENABLE_DYNAMIC 1

#include <ThingsBoard.h>

// Wi-Fi name office-A9 Svnlog@2023
#define WIFI_SSID "WH-A9"
// Password of Wi-Fi.
#define WIFI_PASSWORD "Svnlog@2022"
// Device Aisle 1's token
#define TOKEN  "rJVt2qlkMq0T2xyn0rrz"
// Thingsboard we want to establish a connection too
#define THINGSBOARD_SERVER "demo.thingsboard.io"
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port.
#define THINGSBOARD_PORT  1883U

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
#define MAX_MESSAGE_SIZE  256U

// Baud rate for the debugging serial connection.
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
#define SERIAL_DEBUG_BAUD  115200U

// Initialize underlying client, used to establish a connection
WiFiClient wifiClient;
// Initialize ThingsBoard instance with the maximum needed buffer size
ThingsBoard tb(wifiClient, MAX_MESSAGE_SIZE);

//Replace with ESP receiver broadcoast address.
//MAC address of the head of the rack esp8266
// uint8_t broadcastAddress[] = {0x48, 0x55, 0x19, 0xC1, 0xA9, 0xB1};
//MAC address of the end of the rack esp8266
// uint8_t broadcastAddress[] = {0xEC, 0xFA, 0xBC, 0x45, 0x54, 0x84};

//Replace with ESP receiver broadcoast address.
//MAC address of the head of the rack esp8266
uint8_t broadcastAddress[] = {0x48, 0x55, 0x19, 0xE4, 0xAF, 0xF2};
//MAC address of the end of the rack esp8266
// uint8_t broadcastAddress[] = {0x48, 0x55, 0x19, 0xE4, 0x4F, 0xD1};

const int led = 5; //LED pin GPIO 5.
const int buttonIn = 4; //Push Button connect to pin.
const int buttonOut = 14;
int signalIn = 0;
int signalOut = 0;
int countPeople = 0;
int countPeople_temp = 0;
//Initial variable to be sent
// int status_light = 0;
// int status_temp = 0;
// //Initial variable to be receieved
int direction_rec = 0;
int countPeople_rec = 0;
int direction_temp = 0;

typedef struct struct_message{
  int direction;
  int countPeople;
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
  direction_rec = incomingButtonSig.direction;
  countPeople_rec = incomingButtonSig.countPeople;
}

//Initalizes WiFi connection
void InitWiFi(){
  Serial.print("Connecting to AP ...");
  //Connecting to given WiFi name and password
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    //delay 500ms until connection is successful 
    delay(500);
    Serial.println(".");
  }
  Serial.println("Connected to AP.");
}

//Reconnects the WiFi uses InitWiFi if the connection has been removed
const bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void connectDashboard(){
  
  if(!tb.connected()){
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if(!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)){
      Serial.println("Failed to connect");
      return;
    }
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(led, OUTPUT); //Declare LED as output.
  pinMode(buttonIn, INPUT); //Declare button as input.
  pinMode(buttonOut, INPUT);
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

  // InitWiFi();
}

void loop() {

  signalIn = digitalRead(buttonIn);
  signalOut = digitalRead(buttonOut);
  
  //Check WiFi connection
  // if(!reconnect()){
  //   return;
  // }
  
  if(signalIn == HIGH){
    Serial.println("Walking In");
    countPeople++;
    Serial.println(countPeople);
    digitalWrite(led, HIGH);
    delay(500);
    readButtonSig.direction = 1;
    // direction_rec = 1;
    // countPeople_rec = countPeople;
    readButtonSig.countPeople = countPeople;
    esp_now_send(broadcastAddress, (uint8_t *) &readButtonSig, sizeof(readButtonSig));
  }
  
  if(signalOut == HIGH){
    if(countPeople > 0){
      Serial.println("Walking Out");
      countPeople--;
      Serial.println(countPeople);
      if(countPeople == 0){
        digitalWrite(led, LOW);
      }
      delay(500);
      readButtonSig.direction = 0;
      // direction_rec = 0;
      // countPeople_rec = countPeople;
      readButtonSig.countPeople = countPeople;
      esp_now_send(broadcastAddress, (uint8_t *) &readButtonSig, sizeof(readButtonSig));
    }
  }

  if(direction_temp != direction_rec || countPeople_temp != countPeople_rec){
    countPeople = countPeople_rec;
    direction_temp = direction_rec;
    countPeople_temp = countPeople_rec;
    if(direction_rec == 1){
      Serial.println("Recive signal people walking in");
      // countPeople++;
      Serial.println(countPeople);
      digitalWrite(led, HIGH);
      delay(500);
    }else{
      if(countPeople > 0){
        Serial.println("Recive signal people walking out");
        // countPeople--;
        Serial.println(countPeople);
        if(countPeople == 0){
          digitalWrite(led, LOW);
        }
        delay(500);
      }
    }
  }

  // if(status_temp != status_light){
  //   status_temp = status_light;
  //   connectDashboard();
  //   Serial.println("Sending data...");
  //   if(status_temp == 0){
  //     Serial.println(status_temp);
  //     tb.sendAttributeString("Status", "Avaliable");
  //   }else{
  //     Serial.println(status_temp);
  //     tb.sendAttributeString("Status", "Occupied");
  //   }
  // }
}
