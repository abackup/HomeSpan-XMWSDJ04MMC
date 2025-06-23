#pragma once
#include "HomeSpan.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

#define XIAOMI_LOOP_TIME 7200000  // 温湿度计2小时更新一次

#define BLE_DEVICE_ADDRESS "**:**:**:**:**:**"
#define SERVICE_UUID "ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6"
#define CHARACTERISTIC_UUID "ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6"

float xiaomiTemp = 0, xiaomiHum = 0, xiaomiVolt = 3;
BLEClient *pClient = nullptr;
BLERemoteCharacteristic *pRemoteCharacteristic = nullptr;

enum BLEState
{
  BLE_DISCONNECTED,
  BLE_CONNECTING,
  BLE_CONNECTED
};
BLEState bleState = BLE_DISCONNECTED;

bool connectBLE()
{
  pClient = BLEDevice::createClient();
  if (!pClient->connect(BLEAddress(BLE_DEVICE_ADDRESS)))
    return false;

  auto service = pClient->getService(BLEUUID(SERVICE_UUID));
  if (!service)
    return false;

  pRemoteCharacteristic = service->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID));
  if (!pRemoteCharacteristic || !pRemoteCharacteristic->canRead())
    return false;

  return true;
}

bool readSensor(float &temperature, float &humidity, float &voltage)
{
  if (!pRemoteCharacteristic)
    return false;

  String value = pRemoteCharacteristic->readValue();
  if (value.length() != 6)
    return false;

  const uint8_t *data = (const uint8_t *)value.c_str();
  temperature = ((data[1] << 8) | data[0]) / 10.0;
  humidity = ((data[3] << 8) | data[2]) / 10.0;
  voltage = ((data[5] << 8) | data[4]) / 1000.0;

  return true;
}

int voltageToPercentage(float voltage)
{
  if (voltage >= 3.0)
    return 100;
  if (voltage <= 2.5)
    return 0;
  return (int)((voltage - 2.5) * 200); // 线性插值
}

struct DEV_xiaomiTemp : Service::TemperatureSensor
{
  SpanCharacteristic *temp;
  SpanCharacteristic *statusActive;

  DEV_xiaomiTemp() : Service::TemperatureSensor()
  {
    readSensor(xiaomiTemp, xiaomiHum, xiaomiVolt);
    temp = new Characteristic::CurrentTemperature(xiaomiTemp, true);
    statusActive = new Characteristic::StatusActive(1, true); // 1表示在运行，0表示未在运行
    temp->setRange(-10, 45);
  }
  void loop() override
  {

    if (temp->timeVal() > XIAOMI_LOOP_TIME)
    {
      switch (bleState)
      {
      case BLE_DISCONNECTED:
        bleState = BLE_CONNECTING;
        break;

      case BLE_CONNECTING:
        if (connectBLE())
        {
          bleState = BLE_CONNECTED;
        }
        else
        {
          // 连接失败，稍后重试
          static unsigned long lastRetry = 0;
          if (millis() - lastRetry > 10000)
          {
            lastRetry = millis();
            bleState = BLE_DISCONNECTED;
          }
        }
        break;

      case BLE_CONNECTED:
        if (!pClient->isConnected())
        {
          bleState = BLE_DISCONNECTED;
          break;
        }

        readSensor(xiaomiTemp, xiaomiHum, xiaomiVolt);

        break;
      }
      temp->setVal(xiaomiTemp);
    }
  }
};

struct DEV_xiaomiHum : Service::HumiditySensor
{
  SpanCharacteristic *hum;
  SpanCharacteristic *statusActive;

  DEV_xiaomiHum() : Service::HumiditySensor()
  {

    hum = new Characteristic::CurrentRelativeHumidity(xiaomiHum, true);
    statusActive = new Characteristic::StatusActive(1, true); // 1表示在运行，0表示未在运行
    hum->setRange(0, 100);
  }

  void loop() override
  {

    if (hum->timeVal() > XIAOMI_LOOP_TIME + 1000) // 6秒后更新湿度
    {
      hum->setVal(xiaomiHum);
    }
  }
};

// struct DEV_XiaomiBattery : Service::BatteryService
// {
//   SpanCharacteristic *batteryLevel;
//   SpanCharacteristic *chargingState;
//   SpanCharacteristic *statusLowBattery;

//   DEV_XiaomiBattery() : Service::BatteryService()
//   {

//     batteryLevel = new Characteristic::BatteryLevel(voltageToPercentage(xiaomiVolt), true);
//     chargingState = new Characteristic::ChargingState(1, true);
//     statusLowBattery = new Characteristic::StatusLowBattery(0, true); // 0表示电池正常，1表示电池低电量
//     batteryLevel->setRange(0, 100);
//   }

//   void loop() override
//   {

//     if (batteryLevel->timeVal() > XIAOMI_LOOP_TIME + 1000) // 6秒后更新湿度
//     {
//       batteryLevel->setVal(voltageToPercentage(xiaomiVolt));
//       if (voltageToPercentage(xiaomiVolt) < 20) // 电池电量低于20%
//       {
//         statusLowBattery->setVal(1); // 1表示电池低电量
//       }
//       else
//       {
//         statusLowBattery->setVal(0); // 0表示电池正常
//       }
//     }
//   }
// };
