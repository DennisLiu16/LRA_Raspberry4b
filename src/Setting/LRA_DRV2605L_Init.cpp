#include <iostream>
#include <DRV2605L/LRA_DRV2605L.h>

using namespace LRA_DRV2605L; 

int main(){
    cout << "Init start" << endl;
    /*create LRA driver instance*/
    DRV2605L x_driver(SLAVE_DEFAULT_ID);
    x_driver.init();
    /*read recursive test*/
    int count = 0;
    for(int addr = 0; addr < 0x25 ; addr++)
    {
        uint8_t c;
        c = x_driver.read(addr);
        cout << c << ' ';
        count ++;
        if(count >= 15)
        {
            cout << endl;
            count = 0;
        }
        
    }

        cout << "Init end" << endl;
}
