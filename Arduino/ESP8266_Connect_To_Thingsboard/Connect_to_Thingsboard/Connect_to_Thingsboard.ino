#include <ESP8266WiFi.h>

#define THINGSBOARD_ENABLE_PROGMEM 0

#define THINGSBOARD_ENABLE_PSRAM 0
#define THINGSBOARD_ENABLE_DYNAMIC 1

#include <ThingsBoard.h>

// Wi-Fi name
#define WIFI_SSID "Wifi name"
// Password of Wi-Fi.
#define WIFI_PASSWORD "password"
// Device Aisle 1's token
#define TOKEN  "Access Token"
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

const int signal = 5;
int temp = 0;

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

//Initalizes WiFi connection
void InitWiFi(){
  Serial.println("Connecting to AP ...");
  //Connecting to given WiFi name and password
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    //delay 500ms until connection is successful 
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP.");
}

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  pinMode(signal, INPUT);
  InitWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  temp = digitalRead(signal);
  //Check WiFi connection
  if(!reconnect()){
    return;
  }

  if(temp == HIGH){
    //Connecting to Dashboard using ThingsBoard tool.
    connectDashboard();

    Serial.println("Sending data...");

    tb.sendAttributeString("Status", "Occupied");
  }else{
    connectDashboard();

    Serial.println("Sending data...");

    tb.sendAttributeString("Status", "Avaliable");
  }
}
