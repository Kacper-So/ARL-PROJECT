#include "deviceManager.hpp"

RTC_DATA_ATTR esp_sleep_source_t WakeUpSource;

static unsigned long int currentTime = micros();
static unsigned long int prevTime = 0 ;
static unsigned long int dt = (currentTime - prevTime); 
static unsigned long int loopTime = LOOP_TIME * 1000UL;
static unsigned long int loopTimeWarning = loopTime + (unsigned long)((float)loopTime*0.95);

updateStatus updateStatus_ = UPDATE_OK;

void handle_stream(){
	if(deviceManager.stream_status){
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
	} else {
		deviceManager.server->send(503, "", "");
	}
}

void setup() {	
	initStatus initStatus_ = deviceManager.init();
	switch (initStatus_){
		case INIT_OK:
			log_i("Init performed corectly");
			break;
		case CAM_ERROR:
			while(1) log_e("Camera init error");
			break;
		case SERVER_ERROR:
			while(1) log_e("Server init error");
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
				while(1) log_e("None state error");
				break;
			case GOTOSLEEP_STATE_ERROR:
				while(1) log_e("Go to sleep state error");
				break;
			case STREAM_STATE_ERROR:
				while(1) log_e("Stream state error");
				break;
			default:
				break;
		}
		deviceManager.server->handleClient();
	}
}