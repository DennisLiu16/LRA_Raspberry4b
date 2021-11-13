#include <ErrorCode/LRA_ErrorCode.h>

namespace LRA_Error{

    const char* Error::getErrorName(ErrorType errorId){
        /*const char* err = "a const string" is ok*/
        // wrong: const char* err = var_string; ->error
        const char* errorName = "ERR_UNKNOWN";
        switch(errorId){
            // PI
            case ERR_PI_OK:errorName = "ERR_PI_OK";break;

            //PI_I2C
            //case ERR_PI_I2C_CONNECTION_FAILURE:errorName = "ERR_PI_I2C_CONNECTION_FAILURE";break;   
            // case ERR_PI_I2C_IOCTL_PERMISSION_DENYIED:errorName = "ERR_PI_I2C_IO_PERMISSION_DENYIED";break;
            // case ERR_PI_I2C_READ_FAILURE:errorName = "ERR_PI_I2C_READ_FAILURE";break;

            //PI_DATABASE
            case ERR_PI_DATABASE_CONNETION_FAILURE:errorName = "ERR_PI_DATABASE_CONNETION_FAILURE";break;

            // DRV2605L
            case ERR_DRV2605L_OK:errorName = "ERR_DRV2605L_OK";break;
            case ERR_DRV2605L_READ_FAILURE:errorName = "ERR_DRV2605L_READ_FAILURE";break;
            case ERR_DRV2605L_WRITE_FAILURE:errorName = "ERR_DRV2605L_WRITE_FAILURE";break;
            case ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH:errorName = "ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH";break;
            case ERR_DRV2605L_SLAVE_ID_DISMATCH:errorName = "ERR_DRV2605L_SLAVE_ID_DISMATCH";break;
            case ERR_DRV2605L_OPEN_PORT_FAILURE:errorName = "ERR_DRV2605L_OPEN_PORT_FAILURE";break;

            // SENSOR
            case ERR_SENSOR_OK:errorName = "ERR_SENSOR_OK:errorName";break;

            default:errorName = "ERR_Unknown";break;
        }
        return errorName;
    }

}