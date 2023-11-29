#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_sleep.h"
#include "esp32cam.h"
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


        initStatus init();
        functionStatus cameraInit();
        functionStatus wifiInit();

        updateStatus update();

        functionStatus none_update();
        functionStatus goToSleep_update();
        functionStatus stream_update();

        deviceAction currentAction = none;
        void transitionTo(deviceAction newAction);
        void stateMachine();

        camera_config_t config;
        bool stream_status = false;
        int imgType = 0;
        esp32cam::Resolution res = esp32cam::Resolution::find(160, 120);
        WebServer *server;
};

extern DeviceManager deviceManager;

#endif //__DEVICE_MANAGER_H__