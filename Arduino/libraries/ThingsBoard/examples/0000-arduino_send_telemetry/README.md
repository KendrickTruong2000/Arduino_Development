# Sending telemetry data

## Devices
| Supported Devices |
|-------------------|
|  Arduino Uno      |
|  ESP8266          |

## ThingsBoard API
[Telemetry](https://thingsboard.io/docs/user-guide/telemetry/)

## Feature
Allows uploading telemetry values to the cloud, compared to attributes
these values keep track of their previous values meaning we can draw graphs with them.
Meant for values which change over time and where a history might be useful (temperature, humidity, ...)
