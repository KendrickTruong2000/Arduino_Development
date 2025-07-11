#ifdef ESP8266
#include <ESP8266WiFi.h>
// Disable PROGMEM because the ESP8266WiFi library,
// does not support flash strings.
#define THINGSBOARD_ENABLE_PROGMEM 0
#else
#ifdef ESP32
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif // ESP32
#endif // ESP8266


// Sending data can either be done over MQTT and the PubSubClient
// or HTTPS and the HTTPClient, when using the ESP32 or ESP8266
#define USING_HTTPS false

// Whether the given script is using encryption or not,
// generally recommended as it increases security (communication with the server is not in clear text anymore),
// it does come with an overhead tough as having an encrypted session requires a lot of memory,
// which might not be avaialable on lower end devices.
#define ENCRYPTED false

// Enables sending messages that are bigger than the predefined message size,
// where the message will be sent byte by byte as a fallback instead.
// Requires an additional library, see https://github.com/bblanchon/ArduinoStreamUtils for more information.
#define THINGSBOARD_ENABLE_STREAM_UTILS 1


#if USING_HTTPS
#include <ThingsBoardHttp.h>
#else
#include <ThingsBoard.h>
#endif


// PROGMEM can only be added when using the ESP32 WiFiClient,
// will cause a crash if using the ESP8266WiFiSTAClass instead.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char WIFI_SSID[] PROGMEM = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] PROGMEM = "YOUR_WIFI_PASSWORD";
#else
constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
#endif

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char TOKEN[] PROGMEM = "YOUR_DEVICE_ACCESS_TOKEN";
#else
constexpr char TOKEN[] = "YOUR_DEVICE_ACCESS_TOKEN";
#endif

// Thingsboard we want to establish a connection too
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char THINGSBOARD_SERVER[] PROGMEM = "demo.thingsboard.io";
#else
constexpr char THINGSBOARD_SERVER[] = "demo.thingsboard.io";
#endif

#if USING_HTTPS
// HTTP port used to communicate with the server, 80 is the default unencrypted HTTP port,
// whereas 443 would be the default encrypted SSL HTTPS port
#if ENCRYPTED
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 443U;
#else
constexpr uint16_t THINGSBOARD_PORT = 443U;
#endif
#else
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 80U;
#else
constexpr uint16_t THINGSBOARD_PORT = 80U;
#endif
#endif
#else
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port,
// whereas 8883 would be the default encrypted SSL MQTT port
#if ENCRYPTED
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 8883U;
#else
constexpr uint16_t THINGSBOARD_PORT = 8883U;
#endif
#else
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t THINGSBOARD_PORT PROGMEM = 1883U;
#else
constexpr uint16_t THINGSBOARD_PORT = 1883U;
#endif
#endif
#endif

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint16_t MAX_MESSAGE_SIZE PROGMEM = 128U;
#else
constexpr uint16_t MAX_MESSAGE_SIZE = 128U;
#endif

// Baud rate for the debugging serial connection
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
#if THINGSBOARD_ENABLE_PROGMEM
constexpr uint32_t SERIAL_DEBUG_BAUD PROGMEM = 115200U;
#else
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;
#endif

#if ENCRYPTED
// See https://comodosslstore.com/resources/what-is-a-root-ca-certificate-and-how-do-i-download-it/
// on how to get the root certificate of the server we want to communicate with,
// this is needed to establish a secure connection and changes depending on the website.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char ROOT_CERT[] PROGMEM = R"(-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";
#else
constexpr char ROOT_CERT[] = R"(-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";
#endif
#endif

#if THINGSBOARD_ENABLE_PROGMEM
constexpr char TEMPERATURE_KEY[] PROGMEM = "temperature";
constexpr char HUMIDITY_KEY[] PROGMEM = "humidity";
#else
constexpr char TEMPERATURE_KEY[] = "temperature";
constexpr char HUMIDITY_KEY[] = "humidity";
#endif


// Initialize underlying client, used to establish a connection
#if ENCRYPTED
WiFiClientSecure espClient;
#else
WiFiClient espClient;
#endif
// Initialize ThingsBoard instance with the maximum needed buffer size
#if USING_HTTPS
ThingsBoardHttp tb(espClient, TOKEN, THINGSBOARD_SERVER, THINGSBOARD_PORT);
#else
ThingsBoard tb(espClient, MAX_MESSAGE_SIZE);
#endif


/// @brief Initalizes WiFi connection,
// will endlessly delay until a connection has been successfully established
void InitWiFi() {
#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Connecting to AP ..."));
#else
  Serial.println("Connecting to AP ...");
#endif
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been successfully established
    delay(500);
#if THINGSBOARD_ENABLE_PROGMEM
    Serial.print(F("."));
#else
    Serial.print(".");
#endif
  }
#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Connected to AP"));
#else
  Serial.println("Connected to AP");
#endif
#if ENCRYPTED
  espClient.setCACert(ROOT_CERT);
#endif
}

/// @brief Reconnects the WiFi uses InitWiFi if the connection has been removed
/// @return Returns true as soon as a connection has been established again
bool reconnect() {
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
  // If analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  // Initalize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(1000);
  InitWiFi();
}

void loop() {
  delay(1000);

  if (!reconnect()) {
    return;
  }

#if !USING_HTTPS
  if (!tb.connected()) {
    // Reconnect to the ThingsBoard server,
    // if a connection was disrupted or has not yet been established
    Serial.printf("Connecting to: (%s) with token (%s)\n", THINGSBOARD_SERVER, TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
#if THINGSBOARD_ENABLE_PROGMEM
      Serial.println(F("Failed to connect"));
#else
      Serial.println("Failed to connect");
#endif
      return;
    }
  }
#endif

  // Uploads new telemetry to ThingsBoard using HTTP.
  // See https://thingsboard.io/docs/reference/http-api/#telemetry-upload-api
  // for more details
#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Sending temperature data..."));
#else
  Serial.println("Sending temperature data...");
#endif
  tb.sendTelemetryData(TEMPERATURE_KEY, random(10, 31));

#if THINGSBOARD_ENABLE_PROGMEM
  Serial.println(F("Sending humidity data..."));
#else
  Serial.println("Sending humidity data...");
#endif
  tb.sendTelemetryData(HUMIDITY_KEY, random(40, 90));

#if !USING_HTTPS
  tb.loop();
#endif
}
