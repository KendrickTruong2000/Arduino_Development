# Sending telemetry data

## Devices
| Supported Devices    |
|----------------------|
|  Arduino Uno         |
|  GSM Modem (SIM9000) |

## ThingsBoard API
[Telemetry](https://thingsboard.io/docs/user-guide/telemetry/)

## Feature
Allows uploading telemetry values to the cloud using HTTP (normally MQTT is used), compared to attributes
these values keep track of their previous values meaning we can draw graphs with them.
Meant for values which change over time and where a history might be useful (temperature, humidity, ...)