#include <LRA_ErrorCode.h>

namespace LRA_Error{

    const char* Error::getErrorName(ErrorType errorId){
        const char* errorName = "ERR_UNKNOWN";
        switch(errorId){
            // PI
            case ERR_PI_OK:errorName = "ERR_PI_OK";break;
            case ERR_PI_I2C_CONNECTION_FAILURE:errorName = "ERR_PI_I2C_CONNECTION_FAILURE";break;

            // DRV2605L
            case ERR_DRV2605L_OK:errorName = "ERR_DRV2605L_OK";break;

            // SENSOR
            case ERR_SENSOR_OK:errorName = "ERR_SENSOR_OK:errorName";break;
        }
        return errorName;
    }
    
}