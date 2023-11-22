#ifndef __DATA_STRUCTS_H__
#define __DATA_STRUCTS_H__

typedef enum{
    INIT_OK,
    CAM_ERROR,
    SERVER_ERROR
} initStatus;

typedef enum{
    FUNC_OK,
    FUNC_NOK
} functionStatus;

typedef enum{
    UPDATE_OK,
    NONE_STATE_ERROR,
    GOTOSLEEP_STATE_ERROR,
    STREAM_STATE_ERROR
} updateStatus;

typedef enum{
    none,
    goToSleep,
    stream
} deviceAction;

#endif // __DATA_STRUCTS_H__
