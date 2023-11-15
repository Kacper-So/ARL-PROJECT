// #include "deviceManager.hpp"

// RTC_DATA_ATTR esp_sleep_source_t WakeUpSource;

// static unsigned long int currentTime = micros();
// static unsigned long int prevTime = 0 ;
// static unsigned long int dt = (currentTime - prevTime); 
// static unsigned long int loopTime = LOOP_TIME * 1000UL;
// static unsigned long int loopTimeWarning = loopTime + (unsigned long)((float)loopTime*0.95);

// updateStatus updateStatus_ = UPDATE_OK;


// void setup() {	
// 	initStatus initStatus_ = deviceManager.init();
// 	switch (initStatus_){
// 		case INIT_OK:
// 			log_i("Init performed corectly");
// 			break;
// 		case CAM_ERROR:
// 			log_e("Camera init error");
// 			while(1);
// 			break;
// 		case SD_CARD_ERROR:
// 			log_e("SD card init error");
// 			while(1);
// 			break;
// 		case SERVER_ERROR:
// 			log_e("Server init error");
// 			while(1);
// 			break;
// 		default:
// 			break;
// 	}
// }

// void loop() {
// 	currentTime = micros();
// 	dt = currentTime - prevTime; 
// 	if(dt>=loopTime){
// 		if (dt > loopTimeWarning){
// 			log_w("Loop time %u limited to %u microseconds.\n",dt,loopTimeWarning);
// 		}
// 		prevTime = currentTime; 

		
// 		updateStatus_ = deviceManager.update();
// 		switch (updateStatus_){
// 			case UPDATE_OK:
// 				break;
// 			case NONE_STATE_ERROR:
// 				log_e("None state error");
// 				while(1);
// 				break;
// 			case TAKEPHOTO_STATE_ERROR:
// 				log_e("Take photo state error");
// 				while(1);
// 				break;
// 			case GOTOSLEEP_STATE_ERROR:
// 				log_e("Go to sleep state error");
// 				while(1);
// 				break;
// 			case STREAM_STATE_ERROR:
// 				log_e("Stream state error");
// 				while(1);
// 				break;
// 			default:
// 				break;
// 		}
// 	}
// }

#include "esp_camera.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino.h>
//
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
//
// #define i_led 4
//
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39

#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13
//
//
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);
//
uint8_t cam_num;
bool ws_on = false;
bool cam_on = false;
bool led = false;
framesize_t res = FRAMESIZE_VGA;
//
//wifi//
const char* ssid = "realme 8 Pro";
const char* password = "2sj3wxae";
/////
//
StaticJsonDocument<200> doc_rx;
//
void configCamera(){
  camera_config_t config;
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
  config.pixel_format = PIXFORMAT_JPEG;
//
  config.frame_size = res;
  config.jpeg_quality = 10;
  config.fb_count = 3;
//
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
//
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
//
void setup() {
  //
//   pinMode(i_led,OUTPUT);
  //
  Serial.begin(115200);
//
  configCamera();
  //
  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    // digitalWrite(i_led,0);
    delay(450);
    Serial.print(".");
    // digitalWrite(i_led,1);
    delay(50);
    }
  Serial.println("");
  Serial.print("Connected to network with IP address: ");
  Serial.println(WiFi.localIP());
//
    /* Serial.print("Configuring access point...");
     WiFi.softAP(ssid, password);
     IPAddress myIP = WiFi.softAPIP();
     digitalWrite(i_led,1);
     delay(500);
     digitalWrite(i_led,0);
     Serial.print("AP IP address: ");
     Serial.println(myIP);*/
//
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  //
  Serial.println("Web Server in SETUP");
  server.on("/", handle_onConnect);
  server.onNotFound(srv_handle_not_found);
  server.begin();
  Serial.println("server started successfuly");
  
//   digitalWrite(i_led,0);
}
void loop() {
  server.handleClient();
  webSocket.loop();
  flasher();
  if(cam_on == true && ws_on == true){
    liveCam(cam_num);
  }
}