// construct ref : https://www.twblogs.net/a/5bf2dd96bd9eee0405186df9

#ifndef _LRA_ERRORCODE_H_
#define _LRA_ERRORCODE_H_

namespace LRA_Error
{
    /*>> or << 4 means shift 4 bits in hex*/
    const unsigned int DEVICE_MASK     = 0x8000; 

    /*Error code: 0xAAAABBBBB, AAAA->device id,BBBB->error code*/
    const unsigned int BASIC           = 0x0000;
    const unsigned int DEVICE_PI       = 0x0001;
    const unsigned int DEVICE_DRV2605L = 0x0002;
    const unsigned int DEVICE_SENSOR   = 0x0003;

    /*Error code operation*/
    #define MAKE_ERROR(device,code) ((device << 16) | code)

    #define GET_ERROR_CODE(err) (static_cast<short>(err))
    #define GET_ERROR_DEVICE(err) (static_cast<short> (err>>16))

    #define ERROR_IS_BASIC(err) ((err>>16) == BASIC)
    #define ERROR_DEVICE_IS_PI(err) ((err>>16) == DEVICE_PI)
    #define ERROR_DEVICE_IS_DRV2605L(err) ((err>>16) == DEVICE_DRV2605L)
    #define ERROR_DEVICE_IS_SENSOR(err) ((err>>16) == DEVICE_SENSOR)
   

    /*Types of error*/
    typedef enum{
        //basic error code
        ERR_OK                                 = 0x0000,
        ERR_UNKNOWN                            = 0x0001,
        ERR_OPEN_FAILURE                       = 0x0002,

        //PI
        ERR_PI_OK                              = MAKE_ERROR(DEVICE_PI,ERR_OK),
        ERR_PI_I2C_CONNECTION_FAILURE          = MAKE_ERROR(DEVICE_PI,ERR_OPEN_FAILURE),

        //DRV2605L
        ERR_DRV2605L_OK                        = MAKE_ERROR(DEVICE_DRV2605L,ERR_OK),

        //SENSOR
        ERR_SENSOR_OK                          = MAKE_ERROR(DEVICE_SENSOR,ERR_OK)

    } ErrorType;

    class Error{
        public:
            static const char* getErrorName(ErrorType err);
            
    
    };
}

# endif