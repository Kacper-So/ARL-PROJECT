#include "deviceManager.hpp"

DeviceManager :: DeviceManager(){

}

DeviceManager :: ~DeviceManager(){
    
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
    Serial.println("none update");
    return FUNC_OK;
}

functionStatus DeviceManager:: takePhoto_update(){
    Serial.println("takePhoto update");
    return FUNC_OK;
}

functionStatus DeviceManager:: goToSleep_update(){
    Serial.println("goToSleep update");
    return FUNC_OK;
}

functionStatus DeviceManager:: stream_update(){
    Serial.println("stream update");
    return FUNC_OK;
}