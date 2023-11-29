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
    cfg.setResolution(res);
    cfg.setBufferCount(10);
    if(imgType == 0) cfg.setJpeg(50);
    if(imgType == 1) cfg.setRgb();
    if(imgType == 2) cfg.setYuv();
    if(imgType == 3) cfg.setGrayscale();

    bool ok = esp32cam::Camera.begin(cfg);
    if (!ok) {
        return FUNC_NOK;
    }
    return FUNC_OK;
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
    if (Serial.available() > 0) {
        String receivedMessage = Serial.readStringUntil('\n');

        String command = receivedMessage.substring(0, 3);
        int argument = receivedMessage.substring(4).toInt();

        if (command == "STR") {
            deviceManager.stream_status = true;
            Serial.println("stream on");
            transitionTo(stream);
        } else if (command == "STP"){
            deviceManager.stream_status = false;
            Serial.println("stream off");
            transitionTo(none);
        } else if (command == "SLP"){
            transitionTo(goToSleep);
        } else if (command == "RES"){
            if (!deviceManager.stream_status){
                switch (argument){
                case 1:
                    res = esp32cam::Resolution::find(96, 96);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 2:
                    res = esp32cam::Resolution::find(160, 120);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 3:
                    res = esp32cam::Resolution::find(176, 144);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 4:
                    res = esp32cam::Resolution::find(240, 176);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 5:
                    res = esp32cam::Resolution::find(240, 240);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 6:
                    res = esp32cam::Resolution::find(320, 240);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 7:
                    res = esp32cam::Resolution::find(400, 296);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 8:
                    res = esp32cam::Resolution::find(480, 320);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 9:
                    res = esp32cam::Resolution::find(640, 480);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 10:
                    res = esp32cam::Resolution::find(800, 600);
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                default:
                    Serial.println("ERR");
                    break;
                }
            } else {
                Serial.println("ERR");
            }
        } else if (command == "FMT"){
            if (!deviceManager.stream_status){
                switch (argument){
                case 1:
                    imgType = 0;
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 2:
                    imgType = 1;
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 3:
                    imgType = 2;
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 4:
                    imgType = 3;
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                case 5:
                    imgType = 4;
                    esp32cam::Camera.end();
                    cameraInit();
                    break;
                default:
                    Serial.println("ERR");
                    break;
                }
            } else {
                Serial.println("ERR");
            }
        } else {
            Serial.println("ERR");
        }
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
    return FUNC_OK;
}

DeviceManager deviceManager = DeviceManager();