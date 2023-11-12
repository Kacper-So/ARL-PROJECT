#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "webpage.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <SPIFFS.h>
#include <FS.h>
#include "SD.h"
#include "SPI.h"
#include "defines.hpp"
#include "structs.hpp"

class DeviceManager{
    public:
        DeviceManager();
        ~DeviceManager();


        initStatus init(unsigned int serialBaut);
        functionStatus cameraInit();
        functionStatus sdCardInit();
        functionStatus serverInit();

        updateStatus update(unsigned long dt);


        WebServer server();
        camera_config_t config;


        deviceAction uartBuffer();
        functionStatus takePhoto();
        functionStatus goToSleep();
        functionStatus startStream();
};

extern DeviceManager deviceManager;

#endif //__DEVICE_MANAGER_H__