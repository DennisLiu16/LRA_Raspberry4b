// This c file to test wireingpi working or not
# include <wiringPi.h>
int main(void){
	wiringPiSetup();
	pinMode(29,OUTPUT);
	
	for(;;){
	  digitalWrite(29,HIGH);delay(3000);
	  digitalWrite(29,LOW);delay(3000);
	}
}
