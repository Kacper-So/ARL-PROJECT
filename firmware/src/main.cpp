#include "deviceManager.hpp"

RTC_DATA_ATTR esp_sleep_source_t WakeUpSource;

static unsigned long int currentTime = micros();
static unsigned long int prevTime = 0 ;
static unsigned long int dt = (currentTime - prevTime); 
static unsigned long int loopTime = LOOP_TIME * 1000UL;
static unsigned long int loopTimeWarning = loopTime + (unsigned long)((float)loopTime*0.95);

updateStatus updateStatus_ = UPDATE_OK;

void handle_stream(){
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    deviceManager.server->send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  deviceManager.server->setContentLength(frame->size());
  deviceManager.server->send(200, "image/jpeg");
  WiFiClient client = deviceManager.server->client();
  frame->writeTo(client);
  Serial.println("frame send");
}

void setup() {	
	initStatus initStatus_ = deviceManager.init();
	switch (initStatus_){
		case INIT_OK:
			log_i("Init performed corectly");
			break;
		case CAM_ERROR:
			log_e("Camera init error");
			while(1);
			break;
		case SERVER_ERROR:
			log_e("Server init error");
			while(1);
			break;
		default:
			break;
	}
  deviceManager.server->on("/cam-lo.jpg", handle_stream);
  deviceManager.server->begin();
}

void loop() {
	currentTime = micros();
	dt = currentTime - prevTime; 
	if(dt>=loopTime){
		if (dt > loopTimeWarning){
			log_w("Loop time %u limited to %u microseconds.\n",dt,loopTimeWarning);
		}
		prevTime = currentTime; 

		
		updateStatus_ = deviceManager.update();
		switch (updateStatus_){
			case UPDATE_OK:
				break;
			case NONE_STATE_ERROR:
				log_e("None state error");
				while(1);
				break;
			case GOTOSLEEP_STATE_ERROR:
				log_e("Go to sleep state error");
				while(1);
				break;
			case STREAM_STATE_ERROR:
				log_e("Stream state error");
				while(1);
				break;
			default:
				break;
		}
		deviceManager.server->handleClient();
	}
}

// #include <esp32cam.h>
// #include <WebServer.h>
// #include <WiFi.h>

// const char* WIFI_SSID = "realme 8 Pro";
// const char* WIFI_PASS = "2sj3wxae";

// WebServer server(80);
// static auto ultloRes = esp32cam::Resolution::find(240, 176);
// static auto loRes = esp32cam::Resolution::find(320, 240);
// static auto hiRes = esp32cam::Resolution::find(800, 600);

// void handleBmp()
// {
//   if (!esp32cam::Camera.changeResolution(loRes)) {
//     Serial.println("SET-LO-RES FAIL");
//   }

//   auto frame = esp32cam::capture();
//   if (frame == nullptr) {
//     Serial.println("CAPTURE FAIL");
//     server.send(503, "", "");
//     return;
//   }
//   Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
//                 static_cast<int>(frame->size()));

//   if (!frame->toBmp()) {
//     Serial.println("CONVERT FAIL");
//     server.send(503, "", "");
//     return;
//   }
//   Serial.printf("CONVERT OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
//                 static_cast<int>(frame->size()));

//   server.setContentLength(frame->size());
//   server.send(200, "image/bmp");
//   WiFiClient client = server.client();
//   frame->writeTo(client);
// }

// void serveJpg()
// {
//   auto frame = esp32cam::capture();
//   if (frame == nullptr) {
//     Serial.println("CAPTURE FAIL");
//     server.send(503, "", "");
//     return;
//   }
//   Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
//                 static_cast<int>(frame->size()));

//   server.setContentLength(frame->size());
//   server.send(200, "image/jpeg");
//   WiFiClient client = server.client();
//   frame->writeTo(client);
// }

// void handleJpgLo()
// {
//   if (!esp32cam::Camera.changeResolution(ultloRes)) {
//     Serial.println("SET-LO-RES FAIL");
//   }
//   serveJpg();
// }

// void handleJpgHi()
// {
//   if (!esp32cam::Camera.changeResolution(hiRes)) {
//     Serial.println("SET-HI-RES FAIL");
//   }
//   serveJpg();
// }

// void handleJpg()
// {
//   server.sendHeader("Location", "/cam-hi.jpg");
//   server.send(302, "", "");
// }

// void handleMjpeg()
// {
//   if (!esp32cam::Camera.changeResolution(hiRes)) {
//     Serial.println("SET-HI-RES FAIL");
//   }

//   Serial.println("STREAM BEGIN");
//   WiFiClient client = server.client();
//   auto startTime = millis();
//   int res = esp32cam::Camera.streamMjpeg(client);
//   if (res <= 0) {
//     Serial.printf("STREAM ERROR %d\n", res);
//     return;
//   }
//   auto duration = millis() - startTime;
//   Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
// }

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println();

//   {
//     using namespace esp32cam;
//     Config cfg;
// 	constexpr esp32cam::Pins Seeed{
//         D0: 15,
//         D1: 17,
//         D2: 18,
//         D3: 16,
//         D4: 14,
//         D5: 12,
//         D6: 11,
//         D7: 48,
//         XCLK: 10,
//         PCLK: 13,
//         VSYNC: 38,
//         HREF: 47,
//         SDA: 40,
//         SCL: 39,
//         RESET: -1,
//         PWDN: -1,
//     };
//     cfg.setPins(Seeed);
//     cfg.setResolution(ultloRes);
//     cfg.setBufferCount(10);
//     cfg.setJpeg(50);

//     bool ok = Camera.begin(cfg);
//     Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
//   }

//   WiFi.persistent(false);
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_SSID, WIFI_PASS);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//   }

//   Serial.print("http://");
//   Serial.println(WiFi.localIP());
//   Serial.println("  /cam.bmp");
//   Serial.println("  /cam-lo.jpg");
//   Serial.println("  /cam-hi.jpg");
//   Serial.println("  /cam.mjpeg");

//   server.on("/cam.bmp", handleBmp);
//   server.on("/cam-lo.jpg", handleJpgLo);
//   server.on("/cam-hi.jpg", handleJpgHi);
//   server.on("/cam.jpg", handleJpg);
//   server.on("/cam.mjpeg", handleMjpeg);

//   server.begin();
// }

// void loop()
// {
//   server.handleClient();
// }