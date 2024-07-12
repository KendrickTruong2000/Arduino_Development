#include <ESP8266WiFi.h>
#include <espnow.h>

#define THINGSBOARD_ENABLE_PROGMEM 0

#define THINGSBOARD_ENABLE_PSRAM 0
#define THINGSBOARD_ENABLE_DYNAMIC 1

#include <ThingsBoard.h>

// Wi-Fi name
#define WIFI_SSID "iphone 11 pro max (3)"
// Password of Wi-Fi.
#define WIFI_PASSWORD "bsy90cgdcsa63"

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
uint8_t broadcastAddress[] = {0xEC, 0xFA, 0xBC, 0x45, 0x54, 0x84};

const int led = 5; //LED pin GPIO 5.
const int button = 4; //Push Button connect to pin.
int temp = 0;
//Initial variable to be sent
int status_light = 0;
int status_temp = 0;
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

//Initalizes WiFi connection
void InitWiFi(){
  Serial.print("Connecting to AP ...");
  //Connecting to given WiFi name and password
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    //delay 500ms until connection is successful 
    delay(500);
    Serial.print(".");
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
  Serial.begin(115200);

  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_AP_STA);
  InitWiFi();

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
  pinMode(button, INPUT); //Declare button as input.
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
      delay(500);
    }else{
      status_light = 0;
      digitalWrite(led, LOW);
      delay(500);
    }

    status_rec = status_light;

    readButtonSig.status = status_light;

    esp_now_send(broadcastAddress, (uint8_t *) &readButtonSig, sizeof(readButtonSig));

  }else{
    if(status_rec == 1){
      status_light = 1;
      digitalWrite(led, HIGH);
    }else{
      status_light = 0;
      digitalWrite(led, LOW);
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
  //   delay(500);
  //   WiFi.mode(WIFI_STA);
  //   WiFi.disconnect();
  // }
}
