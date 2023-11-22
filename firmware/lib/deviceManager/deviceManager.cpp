#include "deviceManager.hpp"

const char* ssid = "realme 8 Pro";
const char* password = "2sj3wxae";

DeviceManager :: DeviceManager(){
    server = new WebServer(80);
}

DeviceManager :: ~DeviceManager(){
    
}

functionStatus DeviceManager :: cameraInit(){
    esp32cam::Config cfg;
    constexpr esp32cam::Pins Seeed{
        D0: 15,
        D1: 17,
        D2: 18,
        D3: 16,
        D4: 14,
        D5: 12,
        D6: 11,
        D7: 48,
        XCLK: 10,
        PCLK: 13,
        VSYNC: 38,
        HREF: 47,
        SDA: 40,
        SCL: 39,
        RESET: -1,
        PWDN: -1,
    };
    cfg.setPins(Seeed);
    cfg.setResolution(esp32cam::Resolution::find(160, 120));
    cfg.setBufferCount(10);
    cfg.setJpeg(50);

    bool ok = esp32cam::Camera.begin(cfg);
    if (!ok) {
        return FUNC_NOK;
    }
    return FUNC_OK;

    // config.ledc_channel = LEDC_CHANNEL_0;
    // config.ledc_timer = LEDC_TIMER_0;
    // config.pin_d0 = Y2_GPIO_NUM;
    // config.pin_d1 = Y3_GPIO_NUM;
    // config.pin_d2 = Y4_GPIO_NUM;
    // config.pin_d3 = Y5_GPIO_NUM;
    // config.pin_d4 = Y6_GPIO_NUM;
    // config.pin_d5 = Y7_GPIO_NUM;
    // config.pin_d6 = Y8_GPIO_NUM;
    // config.pin_d7 = Y9_GPIO_NUM;
    // config.pin_xclk = XCLK_GPIO_NUM;
    // config.pin_pclk = PCLK_GPIO_NUM;
    // config.pin_vsync = VSYNC_GPIO_NUM;
    // config.pin_href = HREF_GPIO_NUM;
    // config.pin_sscb_sda = SIOD_GPIO_NUM;
    // config.pin_sscb_scl = SIOC_GPIO_NUM;
    // config.pin_pwdn = PWDN_GPIO_NUM;
    // config.pin_reset = RESET_GPIO_NUM;
    // config.xclk_freq_hz = 20000000;


    // config.frame_size = FRAMESIZE_UXGA;
    // config.pixel_format = PIXFORMAT_JPEG; // for streaming

    // config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    // config.fb_location = CAMERA_FB_IN_PSRAM;
    // config.jpeg_quality = 12;
    // config.fb_count = 1;

    // esp_err_t err = esp_camera_init(&config);
    // if (err != ESP_OK) {
    //     return FUNC_NOK;
    // }
    // return FUNC_OK;
}

functionStatus DeviceManager :: wifiInit(){
    WiFi.begin(ssid, password);
    Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    Serial.println("");
    Serial.print("Connected to network with IP address: ");
    Serial.println(WiFi.localIP());
    return FUNC_OK;
}


initStatus DeviceManager :: init(){
    Serial.begin(SERIAL_BAUT);
    if(cameraInit() == FUNC_NOK) return CAM_ERROR;
    if(wifiInit() == FUNC_NOK) return SERVER_ERROR;

    return INIT_OK;
}

void DeviceManager :: transitionTo(deviceAction newAction){
    currentAction = newAction;
}

void DeviceManager :: stateMachine(){
    if(Serial.available() == 1){
        transitionTo(stream);
    } else if(Serial.available() == 0) {
        transitionTo(none);
    } else {
        transitionTo(none);
    }
}

updateStatus DeviceManager :: update(){
    switch (currentAction){
        case deviceAction::none:
            if(none_update() == FUNC_NOK){
                return NONE_STATE_ERROR;
            }
            break;
        case deviceAction::goToSleep:
            if(goToSleep_update() == FUNC_NOK){
                return GOTOSLEEP_STATE_ERROR;
            }
            break;
        case deviceAction::stream:
            if(stream_update() == FUNC_NOK){
                return STREAM_STATE_ERROR;
            }
            break;
        default:
            break;
    }
    stateMachine();
    return UPDATE_OK;
}

functionStatus DeviceManager:: none_update(){
    // Serial.println("none update");
    return FUNC_OK;
}

functionStatus DeviceManager:: goToSleep_update(){
    // Serial.println("goToSleep update");
    return FUNC_OK;
}

functionStatus DeviceManager:: stream_update(){
    server->handleClient();
    return FUNC_OK;
}

DeviceManager deviceManager = DeviceManager();