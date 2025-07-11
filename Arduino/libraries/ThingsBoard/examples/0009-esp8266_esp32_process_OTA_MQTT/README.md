# Active OTA firmware update (immediate)

## Devices
| Supported Devices |
|-------------------|
|  ESP32            |
|  ESP8266          |

## ThingsBoard API
[OTA Firmware updates](https://thingsboard.io/docs/user-guide/ota-updates/)
[Shared Attributes](https://thingsboard.io/docs/user-guide/attributes/#shared-attributes)

## Feature
Requests the current firmware shared attributes which then start an OTA firmware update immediately,
if the received shared attributes include the same title but a different version number then the one that was passed initially

## Remarks
Custom Updater implementation can be used to flash any firmware device as long as it supports the STL base functionality.
Simply create an own `IUpdater` implementation similar to `ESP32_Updater` or `ESP8266_Updater`.

[Thinger IO Arduino library](https://github.com/thinger-io/Arduino-Library) can be used as inspiration on how to write the firmware to [`WiFiStorage for Arduino Nano`](https://github.com/thinger-io/Arduino-Library/blob/master/src/ThingerWiFiNINAOTA.h), [`File Storage for Arduino Nano`](https://github.com/thinger-io/Arduino-Library/blob/master/src/ThingerMbedOTA.h) or [`Second Stage Bootloader (SNB) for Arduino MKRNB`](https://github.com/thinger-io/Arduino-Library/blob/master/src/ThingerMKRNBOTA.h)
