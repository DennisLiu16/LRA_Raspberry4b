#include <inc/LRA_ErrorCode.h>

namespace LRA_Error{

    const char* Error::getErrorName(ErrorType errorId){
        const char* errorName = "ERR_UNKNOWN";
        switch(errorId){
            // PI
            case ERR_PI_OK:errorName = "ERR_PI_OK";break;

            //PI_I2C
            case ERR_PI_I2C_CONNECTION_FAILURE:errorName = "ERR_PI_I2C_CONNECTION_FAILURE";break;
            case ERR_PI_I2C_SLAVE_ID_DISMATCH:errorName = "ERR_PI_I2C_SLAVE_ID_DISMATCH";break;

            //PI_DATABASE
            case ERR_PI_DATABASE_CONNETION_FAILURE:errorName = "ERR_PI_DATABASE_CONNETION_FAILURE";break;

            // DRV2605L
            case ERR_DRV2605L_OK:errorName = "ERR_DRV2605L_OK";break;
            case ERR_DRV2605L_READ_FAILURE:errorName = "ERR_DRV2605L_READ_FAILURE";break;
            case ERR_DRV2605L_WRITE_FAILURE:errorName = "ERR_DRV2605L_WRITE_FAILURE";break;

            // SENSOR
            case ERR_SENSOR_OK:errorName = "ERR_SENSOR_OK:errorName";break;
        }
        return errorName;
    }

}