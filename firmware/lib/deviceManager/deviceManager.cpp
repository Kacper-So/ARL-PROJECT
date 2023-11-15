#include "deviceManager.hpp"

const char* ssid = "realme 8 Pro";
const char* password = "2sj3wxae";

const char camhtml [] PROGMEM = 
R"=====(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WiFi CAM</title>
<style>
    *{
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    }
    body{
        background-color: black;
    }
    header{
        display: flex;
        justify-content: center;
        align-items: center;
        padding: 2%;
    }
    #live{
        align-items: center;
        max-width: 90vw;
        max-height: 90vh;
        border: 8px solid white;
    }
    .cam{
        display: flex;
        justify-content: center;
        align-items: center;
    }
    h2{
        font-family: Arial, Helvetica, sans-serif;
        color: white;
        text-align: center;
        margin-top: 5px;
    }
    #cam_led{
        width: 45px;
        height: 45px;
        color: blue;
        font-weight: 800;
        font-size: 16;
        background-color: white;
        border: 2px solid blue;
        border-radius: 20%;
        margin: auto;
    }
    #cam_led:active{
        color: white;
        background-color: blue;
    }
    #cam_on{
        width: 50vw;
        height: 45px;
        font-family: 'Times New Roman', Times, serif;
        font-size: medium;
        font-weight: bolder;
        color: white;
        background-color: rgb(0, 0, 0);
        border: 2px solid white;
        border-radius: 10px;
        margin: auto;
    }
    #cam_on:active{
        background-color: red;
        border: 2px solid red;
    }
    .resf{
        margin: auto;
    }
    #res{
      appearance: none;
      background-color: gray;
      border: 2px solid rgb(255, 255, 255);
      color: white;
      font-size: medium;
      font-weight: bolder;
      text-align: center;
      border-radius: 5px;
      padding: 5px;
    }
    #res:active{
      background-color: blue;
    }
    #res option{
      font-weight: bold;
      color: black;
      background-color: rgba(255, 255, 255, 0.479);
    }
</style>
</head>
<header>
    <form class="resf">
        <select id="res">
            <option value="FRAMESIZE_QQVGA">QQVGA [160x120]</option>
            <option value="FRAMESIZE_HQVGA">HQVGA [240x176]</option>
            <option value="FRAMESIZE_QVGA">QVGA [320x240]</option>
            <option value="FRAMESIZE_CIF" selected>CIF [400x296]</option>
            <option value="FRAMESIZE_VGA">VGA [640x480]</option>
            <option value="FRAMESIZE_SVGA">SVGA [800x600]</option>
            <option value="FRAMESIZE_XGA">XGA [1024x768]</option>
            <option value="FRAMESIZE_SXGA">SXGA [1280x1024]</option>
            <option value="FRAMESIZE_UXGA">UXGA [1600x1200]</option>
        </select>
    </form>
    <button id="cam_on" onclick="camon()">| | WiFi CAM | |</button>
    <button id="cam_led" onclick="ledon()">LED</button>
</header>
<body>
<div class="cam">
<img id='live' src=''>
</div>
</body>
<script>
    var wsc;
function connect_websocket(){
    wsc = new WebSocket('ws://'+ window.location.hostname + ':81/', ['arduino']);
    wsc.onopen = function(){
      wsc.send('connect' + new Date());
    };
    wsc.onerror = function(error){
    console.log('WebSocket Error', error);
    //alert('WebSocket Error', error);
    };
    wsc.binaryType = 'arraybuffer';
    wsc.onmessage = function (msg){
        var bytes = new Uint8Array(msg.data);
        var binary= '';
        var len = bytes.byteLength;
        for (var i = 0; i < len; i++) {
            binary += String.fromCharCode(bytes[i])
        }
        var img = document.getElementById('live');
        img.src = 'data:image/jpg;base64,'+window.btoa(binary);
    }
    wsc.onclose = function(){
       // alert('The connection has been closed.');
    }
}
connect_websocket();
    var res = document.getElementById("res");
    res.addEventListener("input",function (){
        console.log(this.value);
        wsc.send(JSON.stringify({"T":"res","res":this.value}));
    });
    var led = false;
    function ledon(){
        led =! led;
        console.log("flasher = " + led);
        wsc.send(JSON.stringify({"T":"led","led":led}));
    }
    var cam = false;
    function camon(){
        cam =! cam;
        console.log("cam = " + cam);
        wsc.send(JSON.stringify({"T":"cam","cam":cam}));
    }
</script>
</html>
)=====";

WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);
StaticJsonDocument<200> doc_rx;

bool ws_on = false;
bool cam_on = false;
bool led = false;
uint8_t cam_num;

void liveCam(uint8_t num){
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Frame buffer could not be acquired");
      return;
  }
  //replace this with your own function
  webSocket.sendBIN(num, fb->buf, fb->len);
//
  //return the frame buffer back to be reused
  esp_camera_fb_return(fb);
}

void WiFi_Recived_data(){
  String type = doc_rx["T"];
  if(type == "cam"){cam_on = doc_rx["cam"];}
  if(type == "led"){led = doc_rx["led"];}
  //  
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
//
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            ws_on = false;
            break;
        case WStype_CONNECTED:
            Serial.println("Client " + String(num) + " connected");
            cam_num = num;
            ws_on = true;
            break;
        case WStype_TEXT:
            Serial.printf("[%u]-- %s\n", num, payload);
            DeserializationError error = deserializeJson(doc_rx, payload);
             if(error){
               Serial.print(F("deserializeJason() failed: "));
               Serial.println(error.f_str());
               return;
              }
             else{
               WiFi_Recived_data();
             }
             break;
      //
    }
}

void flasher(){
  if(led == true && ws_on == true){
    // digitalWrite(i_led,1);
  }
  if(led == false || ws_on == false){
    // digitalWrite(i_led,0);
  }
}

void handle_onConnect(){
  server.send(200, "text/html", camhtml);
}
void srv_handle_not_found (){
  server.send(404, "text/plain", "File not Found");
}

DeviceManager :: DeviceManager(){

}

DeviceManager :: ~DeviceManager(){
    
}

functionStatus DeviceManager :: cameraInit(){
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG; // for streaming
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        return FUNC_NOK;
    }
    return FUNC_OK;
}

functionStatus DeviceManager :: sdCardInit(){
    return FUNC_OK;
}

functionStatus DeviceManager :: serverInit(){
    WiFi.begin(ssid, password);
    Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
        // digitalWrite(i_led,0);
        delay(450);
        // Serial.print(".");
        // digitalWrite(i_led,1);
        delay(50);
    }
    Serial.println("");
    Serial.print("Connected to network with IP address: ");
    Serial.println(WiFi.localIP());

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("Web Server in SETUP");
    server.on("/", handle_onConnect);
    server.onNotFound(srv_handle_not_found);
    server.begin();
    Serial.println("server started successfuly");
    return FUNC_OK;
}


initStatus DeviceManager :: init(){
    Serial.begin(SERIAL_BAUT);
    if(cameraInit() == FUNC_NOK) return CAM_ERROR;
    if(sdCardInit() == FUNC_NOK) return SD_CARD_ERROR;
    if(serverInit() == FUNC_NOK) return SERVER_ERROR;

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
        case deviceAction::takePhoto:
            if(takePhoto_update() == FUNC_NOK){
                return TAKEPHOTO_STATE_ERROR;
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

functionStatus DeviceManager:: takePhoto_update(){
    // Serial.println("takePhoto update");
    return FUNC_OK;
}

functionStatus DeviceManager:: goToSleep_update(){
    // Serial.println("goToSleep update");
    return FUNC_OK;
}

functionStatus DeviceManager:: stream_update(){
    server.handleClient();
    webSocket.loop();
    flasher();
    if(cam_on == true && ws_on == true){
        liveCam(cam_num);
    }
    return FUNC_OK;
}



DeviceManager deviceManager = DeviceManager();