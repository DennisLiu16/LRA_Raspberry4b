// using spi
// z axis down +

#ifndef _LRA_ADXL355_H_
#define _LRA_ADXL355_H_

#include <PI/LRA_PI_Util.h>
#include <ErrorCode/LRA_ErrorCode.h>
#include <cmath>
#include <deque>
#include <thread>
#include <mutex>
extern "C" {
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <signal.h>
}

/* 
    GETMASK is writemask or readmask depend on what you want to do.
    ==> keep data or read data on certain bits
        uint8_t data = 0b00011011;
                         ^^^^^^^^
                         76543210
        uint8_t wantR3To4Bits = data & GETMASK(2,3);
        wantR3To4Bits = 0b00011000;
                            ==
        you can shift two get value like
        (wantR3To4Bits >> 3) will get 0b00000011

    ----------------------------------------------------------------------
    ==> or you want to overwrite some part of register
        uint8_t data = 0b00011011;
                         ^^^^^^^^
                         76543210
        uint8_t target = (0b1001) << 3;

        // we need to clear 3 to 6 bits first
        data &= ~GETMASK(4,3);
        uint8_t wantW3To6Bits =  (target & GETMASK(4,3)) | data;
*/

namespace LRA_ADXL355
{
    using namespace LRA_PI_Util;
    using namespace LRA_Error;

    class ADXL355{
        public:
        static ADXL355* InstanceArray[10];  // you need to mod if you need more than 10 adxl355 instances

        int SPI_fd = 0;
        double AccMeasureRange = dRange_4g; //+- 4.196g in default --> change to getRange later
        uint8_t buf[4096] = {0};
        timespec adxl355_birth_time;
        volatile bool _fifoINTRdyFlag = 0;

        // ref https://saadquader.wordpress.com/2014/10/19/const-pointer-in-c-or-cplusplus/ -- read only 
        const uint8_t* readBufPtr = buf+1;    // where the read buf start. Read from this address of buf all the time


        typedef struct{
            timespec timestamp;
            int intX;
            int intY;
            int intZ;
        }AccUnit;

        typedef struct fAccUnit
        {
            double time_ms;
            double fX;
            double fY;
            double fZ;

            bool operator==(const fAccUnit& rhs) const
            {
                return ( fX == rhs.fX && fY == rhs.fY && fZ == rhs.fZ && time_ms == rhs.time_ms);
            }

            fAccUnit operator+=(fAccUnit const& rhs ) 
            {
                fX += rhs.fX;
                fY += rhs.fY;
                fZ += rhs.fZ;
                time_ms = (time_ms > rhs.time_ms) ? time_ms : rhs.time_ms;
                return *this;
            }

            fAccUnit operator+(fAccUnit const& rhs)
            {
                return *this+=rhs;
            }

            fAccUnit operator/=(double const& rhs)
            {
                // maybe can be optimized
                fX /= rhs;
                fY /= rhs;
                fZ /= rhs;
                return *this;
            }

            fAccUnit operator/(double const& rhs)
            {
                return *this/=rhs;
            }

        }fAccUnit;

        typedef struct fOffset{
            double fX;
            double fY;
            double fZ;

            bool operator==(const fOffset& rhs) const
            {
                return ( fX == rhs.fX && fY == rhs.fY && fZ == rhs.fZ);
            }


        }fOffset;

        deque<fAccUnit> dq_fAccUnitData;

        const double dRange_2g = 2.048*2;
        const double dRange_4g = 4.096*2;
        const double dRange_8g = 8.192*2;

        enum RW{
            WRITE = 0,
            READ = 1,

            RWByteMax = 4095,
        };

        enum AccDataMarker{
            Err_UnKnown = -2,
            Err_Len2Short = -1,
            isOther = 0,
            isX = 1,
            isEmpty = 2,

            TypeAxes = 0,
            TypeFifo = 1,
    
            XDataMarkerPos = 2,
            DataMarkerLen = 2,

            // length of acc data
            LenDataAxis = 3, // single axis
            LenOffsetSet = 6,
            LenDataSet = 9, // single acc unit
            LenBitAxis = 20,

        };
        
        enum Default{
            spi_speed = 10000000,
            spi_channel = 0,    //CE number
            spi_mode = 0,    //from datasheet

            open_updateThread = 1,

            polling_update_mode = 0,
            INT_update_mode = 1,
            acc_adc_num = 1048576, // (2^20)
            offset_adc_num = 65536,// (2^16)
            temp_adc_num = 4096,   // (2^12)

            MAX_INSTANCE_NUM = 10,

            INT1 = 29,

            AVG_data_size = 10000,
            
        };

        enum Value{


            Range_2g = 0b01,
            Range_4g = 0b10,
            Range_8g = 0b11,


        };

        enum regIndex{
            // 55 regbit
            devid_ad,
            devid_mst,
            partid,
            revid,
            nvm_busy,
            activity,
            fifo_ovr,
            fifo_full,
            data_rdy,
            fifo_entries,
            temp1,
            temp2,
            xdata3,
            xdata2,
            xdata1,
            ydata3,
            ydata2,
            ydata1,
            zdata3,
            zdata2,
            zdata1,
            fifo_data,
            offset_x_h,
            offset_x_l,
            offset_y_h,
            offset_y_l,
            offset_z_h,
            offset_z_l,
            act_z,
            act_y,
            act_x,
            act_thresh_h,
            act_thresh_l,
            act_count,
            hpf_corner,
            odr_lpf,
            fifo_samples,
            act_en2,
            ovr_en2,
            full_en2,
            rdy_en2,
            act_en1,
            ovr_en1,
            full_en1,
            rdy_en1,
            ext_clk,
            ext_sync,
            i2c_hs,
            INT_POL,
            range,
            drdy_off,
            temp_off,
            standby,
            st2,
            st1,
            reset,
            x_axis_marker,
            empty_indicator,
            reg_num = 58
        };

        enum class Addr{  //register address
            DEVID_AD = 0x00,
            DEVID_MST = 0x01,
            PARTID = 0x02,
            REVID = 0x03,
            STATUS = 0x04,
            FIFO_ENTRIES = 0x05,
            TEMP2 = 0x06,
            TEMP1 = 0x07,
            XDATA3 = 0x08,
            XDATA2 = 0x09,
            XDATA1 = 0x0A,
            YDATA3 = 0x0B,
            YDATA2 = 0x0C,
            YDATA1 = 0x0D,
            ZDATA3 = 0x0E,
            ZDATA2 = 0x0F,
            ZDATA1 = 0x10,
            FIFO_DATA = 0x11,
            OFFSET_X_H = 0x1E,
            OFFSET_X_L = 0x1F,
            OFFSET_Y_H = 0x20,
            OFFSET_Y_L = 0x21,
            OFFSET_Z_H = 0x22,
            OFFSET_Z_L = 0x23,
            ACT_EN = 0x24,
            ACT_THRESH_H = 0x25,
            ACT_THRESH_L = 0x26,
            ACT_COUNT = 0x27,
            FILTER = 0x28,
            FIFO_SAMPLES = 0x29,
            INT_MAP = 0x2A,
            SYNC = 0x2B,
            RANGE = 0x2C,
            POWER_CTL = 0x2D,
            SELF_TEST = 0x2E,
            RESET = 0x2F,
        };
        const Addr addr[reg_num]={
            /*devid_ad = */  Addr::DEVID_AD,
            /*devid_mst = */ Addr::DEVID_MST,
            /*partid = */    Addr::PARTID,
            /*revid = */     Addr::REVID,
            
            /*nvm_busy = */  Addr::STATUS,
            /*activity =*/   Addr::STATUS,
            /*fifo_ovr =*/   Addr::STATUS,
            /*fifo_full =*/  Addr::STATUS,
            /*data_rdy = */  Addr::STATUS,

            /*fifo_entries = */Addr::FIFO_ENTRIES,
            /*temp1 = */Addr::TEMP1,
            /*temp2 = */Addr::TEMP2,
            
            /*xdata3 = */Addr::XDATA3,
            /*xdata2 = */Addr::XDATA2,
            /*xdata1 = */Addr::XDATA1,
            /*ydata3 = */Addr::YDATA3,
            /*ydata2 = */Addr::YDATA2,
            /*ydata1 = */Addr::YDATA1,
            /*zdata3 = */Addr::ZDATA3,
            /*zdata2 = */Addr::ZDATA2,
            /*zdata1 = */Addr::ZDATA1,

            /*fifo_data = */Addr::FIFO_DATA,

            /*offset_x_h = */Addr::OFFSET_X_H,
            /*offset_x_l = */Addr::OFFSET_X_L,
            /*offset_y_h = */Addr::OFFSET_Y_H,
            /*offset_y_l = */Addr::OFFSET_Y_L,
            /*offset_z_h = */Addr::OFFSET_Z_H,
            /*offset_z_l = */Addr::OFFSET_Z_L,

            /*act_z = */Addr::ACT_EN,
            /*act_y = */Addr::ACT_EN,
            /*act_x = */Addr::ACT_EN,

            /*act_thresh_h = */Addr::ACT_THRESH_H,
            /*act_thresh_l = */Addr::ACT_THRESH_L,

            /*act_count = */Addr::ACT_COUNT,

            /*hpf_corner = */Addr::FILTER,
            /*odr_lpf = */Addr::FILTER,

            /*fifo_samples = */Addr::FIFO_SAMPLES,

            /*act_en2 = */Addr::INT_MAP,
            /*ovr_en2 = */Addr::INT_MAP,
            /*full_en2 = */Addr::INT_MAP,
            /*rdy_en2 = */Addr::INT_MAP,
            /*act_en1 = */Addr::INT_MAP,
            /*ovr_en1 = */Addr::INT_MAP,
            /*full_en1 = */Addr::INT_MAP,
            /*rdy_en1 = */Addr::INT_MAP,

            /*ext_clk = */Addr::SYNC,
            /*ext_sync = */Addr::SYNC,

            /*i2c_hs = */Addr::RANGE,
            /*INT_POL = */Addr::RANGE,
            /*range = */Addr::RANGE,

            /*drdy_off = */Addr::POWER_CTL,
            /*temp_off = */Addr::POWER_CTL,
            /*standby = */Addr::POWER_CTL,

            /*st2 = */Addr::SELF_TEST,
            /*st1 = */Addr::SELF_TEST,

            /*reset = */Addr::RESET,

            /*x-axis marker*/Addr::FIFO_DATA,
            /*empty indicator*/Addr::FIFO_DATA
        };
        const uint8_t startbit[reg_num]={
            /*devid_ad = */0,
            /*devid_mst = */0,
            /*partid = */0,
            /*revid = */0,

            /*nvm_busy = */4,
            /*activity = */3,
            /*fifo_ovr = */2,
            /*fifo_full = */1,
            /*data_rdy = */0,

            /*fifo_entries = */0,
            /*temp2 = */0,
            /*temp1 = */0,

            /*xdata3 = */0,
            /*xdata2 = */0,
            /*xdata1 = */4,
            /*ydata3 = */0,
            /*ydata2 = */0,
            /*ydata1 = */4,
            /*zdata3 = */0,
            /*zdata2 = */0,
            /*zdata1 = */4,
            
            /*fifo_data = */0,

            /*offset_x_h = */0,
            /*offset_x_l = */0,
            /*offset_y_h = */0,
            /*offset_y_l = */0,
            /*offset_z_h = */0,
            /*offset_z_l = */0,

            /*act_z = */2,
            /*act_y = */1,
            /*act_x = */0,

            /*act_thresh_h = */0,
            /*act_thresh_l = */0,

            /*act_count = */0,

            /*hpf_corner = */4,
            /*odr_lpf = */0,

            /*fifo_samples = */0,

            /*act_en2 = */7,
            /*ovr_en2 = */6,
            /*full_en2 = */5,
            /*rdy_en2 = */4,
            /*act_en1 = */3,
            /*ovr_en1 = */2,
            /*full_en1 = */1,
            /*rdy_en1 = */0,

            /*ext_clk = */2,
            /*ext_sync = */0,

            /*i2c_hs =   */7,
            /*int_pol = */6,
            /*range = */0,

            /*drdy_off = */2,
            /*temp_off = */1,
            /*standby = */0,

            /*st2 = */1,
            /*st1 = */0,

            /*reset = */0,

            /*x-axis marker*/0,
            /*empty indicator*/1

        };
        const uint8_t length[reg_num]={
            /*devid_ad = */8,
            /*devid_mst = */8,
            /*partid = */8,
            /*revid = */8,

            /*nvm_busy = */1,
            /*activity = */1,
            /*fifo_ovr = */1,
            /*fifo_full = */1,
            /*data_rdy = */1,

            /*fifo_entries = */7,
            /*temp2 = */4,
            /*temp1 = */8,

            /*xdata3 = */8,
            /*xdata2 = */8,
            /*xdata1 = */4,
            /*ydata3 = */8,
            /*ydata2 = */8,
            /*ydata1 = */4,
            /*zdata3 = */8,
            /*zdata2 = */8,
            /*zdata1 = */4,
            
            /*fifo_data = */8,

            /*offset_x_h = */8,
            /*offset_x_l = */8,
            /*offset_y_h = */8,
            /*offset_y_l = */8,
            /*offset_z_h = */8,
            /*offset_z_l = */8,

            /*act_z = */1,
            /*act_y = */1,
            /*act_x = */1,

            /*act_thresh_h = */8,
            /*act_thresh_l = */8,

            /*act_count = */8,

            /*hpf_corner = */3,
            /*odr_lpf = */4,

            /*fifo_samples = */7,

            /*act_en2 = */1,
            /*ovr_en2 = */1,
            /*full_en2 = */1,
            /*rdy_en2 = */1,
            /*act_en1 = */1,
            /*ovr_en1 = */1,
            /*full_en1 = */1,
            /*rdy_en1 = */1,

            /*ext_clk = */1,
            /*ext_sync = */2,

            /*i2c_hs =   */1,
            /*int_pol = */1,
            /*range = */2,

            /*drdy_off = */1,
            /*temp_off = */1,
            /*standby = */1,

            /*st2 = */1,
            /*st1 = */1,

            /*reset = */8,
            /*x-axis marker*/1,
            /*empty indicator*/1,
        };

        ADXL355(int channel, int speed,int mode,bool updateThread,bool updateMode, void (*isr_handler)(void));   // channel is CE pin index
        ~ADXL355();

        /*Setting related*/

        /**
         * @brief read offset data of 3 axes
         * 
         * @return fOffset 
         */
        fOffset readOffset();

        /**
         * @brief auto set offset, default 10000 samples - tune this at AVG_data_size
         * 
         */
        void setOffset(unsigned int samples);

        void setOffset(fOffset foffset);

        void setAccRange(int range);

        double getAccRange();

        static void isr_default();

        /*Thread safe related*/

        void _updateInBackground();

        /**
         * @brief deque push back for thread safe
         * 
         * @param _accunit 
         */
        void dq_push_back(const fAccUnit _faccunit);

        /**
         * @brief deque pop front for thread safe, get and rm at same time
         * 
         * @return AccUnit 
         */
        fAccUnit dq_pop_front();

        /*Bit Operation related*/

        /**
         * @brief preparse one acc data set (9 bytes) in buf into int
         * 
         * @param buf 
         * @param len
         * @return ssize_t, true if parse successfully, or return false
         */
        ssize_t PreParseOneAccDataUnit(const uint8_t* buf,ssize_t len, int type);

        /**
         * @brief preparse all acc data in buf into int
         * 
         * @param buf 
         * @param len
         * @return ssize_t , return how many groups of AccUnit parsed sucessfully 
         */
        ssize_t PreParseAccData(ssize_t len);

        /**
         * @brief 
         * 
         * @param _accUnit 
         * @param _faccUnit 
         * @return ssize_t 
         */
        ssize_t ParseAccDataUnit(AccUnit* _accUnit, fAccUnit* _faccUnit);

        /**
         * @brief read data from X_axis register to Z_axis register directly
         * 
         * @param tmp_buf
         * @return ssize_t 
         */
        ssize_t readAxesDataOnce(uint8_t* tmp_buf);

        /**
         * @brief read accleration data once, x or y or z total 3 bytes uint8_t should be read to buf
         * 
         * @return ssize_t
         */
        ssize_t readFifoDataOnce();

        /**
         * @brief read accleration data set once, x y z total 9 bytes uint8_t should be read to buf
         * 
         * @details test only -> maybe mess up FIFO order, if you want to get newest certain axis value. You should use readAccX instead 
         * @param buf 
         * @return ssize_t
         */
        ssize_t readFifoDataSetOnce();

        /**
         * @brief overload of given restored buffer(10 byte total)
         * 
         * @param tmp_buf 
         * @return ssize_t 
         */
        ssize_t readFifoDataSetOnce(uint8_t* tmp_buf);

        /**
         * @brief Set register at regaddr to val with no writemask
         * 
         * @param regaddr 
         * @param val 
         */
        void setSingleReg(uint8_t regaddr,uint8_t val);

        /**
         * @brief Set register at regaddr to val with writemask 
         * 
         * @param regaddr 
         * @param val 
         * @param mask 
         */
        void setSingleReg(uint8_t regaddr,uint8_t val,uint8_t mask);

        /**
         * @brief Set single bit pair of regindex to val
         * 
         * @details val is "a whole byte", not bit pair correspond value only, you need to locate correct value in correct place to form val. 
         *          Or use enum reg_val instead.
         * 
         * @param regindex 
         * @param val 
         *  
         */
        void setSingleBitPair(regIndex regindex,uint8_t val);

        /**
         * @brief get bit pair value in "correct position", that is at pos => startbit to (startbit + length) 
         * 
         * @param bIndex 
         * @return uint8_t 
         */
        uint8_t getSingleBitPair(regIndex bIndex);

        /**
         * @brief read reg value at regaddr to buf
         * 
         * @param regaddr 
         * @param buf 
         * @return ssize_t 
         */
        ssize_t readSingleByte(uint8_t regaddr);

        /**
         * @brief read temperature (12bits) to buf
         * 
         * @param buf 
         * @return uint8_t --> change to double instead
         */
        uint8_t readTemp();

        /**
         * @brief read len  bytes or registers that start from regaddr  
         * 
         * @param regaddr 
         * @param buf 
         * @param len 
         * @return ssize_t 
         */
        ssize_t readMultiByte(uint8_t regaddr, ssize_t len);

        /**
         * @brief overload of readMultiByte given restored buffer
         * 
         * @param regaddr 
         * @param len 
         * @param tmp_buf 
         * @return ssize_t 
         */
        ssize_t readMultiByte(uint8_t regaddr, ssize_t len,uint8_t* tmp_buf);

        /**
         * @brief Set multiple bytes data to ADXL355
         * 
         * @param regaddr 
         * @param len 
         * @param tmp_buf 
         * @return ssize_t 
         */
        ssize_t setMultiByte(uint8_t regaddr, ssize_t len,uint8_t* tmp_buf);

        /**
         * @brief if len > 3, check buf[2] data marker.
         * 
         * @details including sho error and empty warning, but not including correct FIFO
         * 
         * @param buf 
         * @return AccDataMarker 
         */
        AccDataMarker CheckDataMarker(ssize_t len);

        //normal setting functions

        /**
         * @brief soft reset this ADXL355 instance
         * 
         * @return * void 
         */
        void resetThisAdxl355();

        /**
         * @brief Get the Stand By State object
         * 
         * @return true , in standby(low power mode)
         * @return false, in measure(high power mode)
         */
        bool getStandByState();

        /**
         * @brief set ADXL355 to standby mode(low power), stop any measurement
         * 
         */
        void setStandByMode();

        /**
         * @brief set ADXL355 to measure mode(high power mode)
         * 
         */
        void setMeasureMode();

        /**
         * @brief 
         * 
         * @return ssize_t 
         */
        uint8_t getPartID();

        /**
         * @brief Get all registers val in buf
         * 
         * @return ssize_t 
         */
        ssize_t getAllReg();

        /**
         * @brief stop _doMeasurement for other SPI operation
         * 
         */
        void StopMeasurement();

        /**
         * @brief recover _doMeasurement
         * 
         */
        void StartMeasurement();


        
        /*Bit function*/
        /**
         * @brief Get bit pair length
         * 
         * @param bIndex 
         * @return uint8_t bit pair length
         */
        uint8_t getLength(regIndex bIndex);

        /**
         * @brief Get bit pair startbit pos
         * 
         * @param bIndex 
         * @return uint8_t startbit pos
         */
        uint8_t getStartBit(regIndex bIndex);

        /**
         * @brief Get bit pair reg addr
         * 
         * @param bIndex 
         * @return uint8_t bit pair reg addr
         */
        uint8_t getAddr(regIndex bIndex);

        /**
         * @brief use uint8_t index to get addr
         * 
         * @param uIndex 
         * @return uint8_t 
         */
        //uint8_t getAddr(uint8_t uIndex);

        protected:
        int _thisInstanceIndex = -1;
        int _channel = 0;
        AccUnit _MyAccUnit;
        fAccUnit _MyfAccUnit;
        bool _updateThread = 0;
        bool _exitThread = 0;
        bool _doMeasurement = 0;
        bool _updateMode = 0;
        std::mutex deque_mutex;
    };
}
#endif

