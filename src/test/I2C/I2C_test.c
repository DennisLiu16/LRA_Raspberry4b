// ref : https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int file_i2c = 0;
int length;
unsigned char buffer[60] = {0};

void main()
{
    /*----- OPEN THE I2C BUS ----*/
    char *filename = (char*)"/dev/i2c-1";
    if (( file_i2c = open(filename,O_RDWR)) < 0){
        printf("fail code : %d\n",file_i2c);
        printf("Failed to open the i2c bus\n");
        return;
    }

    /*----- OPEN SUCCESSED ----*/
    int addr = 0x5a;
    if (ioctl(file_i2c,I2C_SLAVE,addr) < 0){
        printf("Failed to acquire bus access and/or talk to slave.\n");
        return;
    }

    /*----- READ BYTE ----*/
    length = 4;			//<<< Number of bytes to read
    if (read(file_i2c, buffer, length) != length)		//read() returns the number of bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
    {
        //ERROR HANDLING: i2c transaction failed
        printf("Failed to read from the i2c bus.\n");
    }
    else
    {
        printf("Data read: %s\n", buffer);
    }


    //----- WRITE BYTES -----
    // buffer[0] = 0x01;
    // buffer[1] = 0x02;
    // length = 2;			//<<< Number of bytes to write
    // if (write(file_i2c, buffer, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
    // {
    //     /* ERROR HANDLING: i2c transaction failed */
    //     printf("Failed to write to the i2c bus.\n");
    // }
}

