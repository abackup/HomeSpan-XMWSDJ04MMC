[中文文档](./README.md) | [README](./README_en.md)

## DEV_XMWSDJ04MMC
Add [Xiaomi ThermoHygrometer 4](https://home.mi.com/webapp/content/baike/product/index.html?model=miaomiaoce.sensor_ht.t6#/) to HomeSpan to display temperature and humidity on HomeKit.

<p align="center">
<img src="images/XMWSDJ04MMC.png" alt="Xiaomi Thermometer and Hygrometer 4" width="302"/>
</p>

* Download `DEV_XMWSDJ04MMC.h` and put it in the `/ HomeSpan/src/` folder of the HomeSpan library file. Add the following header file to the HomeSpan code:

```C++
#include "DEV_XMWSDJ04MMC.h"
```

* Modify the update time parameter in `DEV_XMWSDJ04MMC.h` (the default is 7200000 milliseconds, that is, update once every 2 hours) and Bluetooth address information.

```C++
#define LOOPTIME 7200000 // Thermohygrometer updates every 2 hours

#define BLE_DEVICE_ADDRESS "**:**:**:**:**:**"
```

* In `the setup()` function, call `DEV_xiaomiTemp()`、`DEV_xiaomiHum()`and `new DEV_XiaomiBattery()` to implement the thermohygrometer and the battery level function. **After HomeSpan is powered on, you need to press the button of Xiaomi Thermohygrometer 4 to pair**. Examples are as follows:  

```C++
#include "HomeSpan.h"
#include "DEV_XMWSDJ04MMC.h"

void setup()
  {
    Serial.begin(115200);

    homeSpan.setWifiCredentials("your WIFI", "WIFI password");

    BLEDevice::init("");

    homeSpan.begin(Category::Bridges, "HomeKit Bridge", "Bridge", "ESP32-C3 mini"); 

    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();

    new SpanAccessory();
    new Service::AccessoryInformation();
    new Characteristic::Identify();

    new DEV_XiaomiTemp();
    new DEV_XiaomiHum();
    new DEV_XiaomiBattery();

//
//    new SpanAccessory();
//    new Service::AccessoryInformation();
//    new Characteristic::Identify();
//    ....

  }

  void loop()
  {

    homeSpan.poll();
    
  }

```
