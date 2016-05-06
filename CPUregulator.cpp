#include <iostream>
#include <csignal>
#ifdef __arm__
    #define RPI
    #include <wiringPi.h>
#endif

using namespace std;

/*
	- shell cmd: /opt/vc/bin/vcgencmd measure_temp
	- shell cmd; cat /sys/class/thermal/thermal_zone0/temp 
*/
//Globals
const int sleepTime = 2500; //In milliseconds [>=2500]
const float coolingTemp = 35.1; //Temperature when start cooling
const int fan_control_pin = 3; // Wiring pi pin
const int controlMode = 3; 
/* 
    1 == temp < coolingTemp //when temp > coolingTemp start cooling until temp < coolingTemp
    2 == temp < coolingTemp && count > minCycles  // 10 times serial temp must be low to turn off
    3 == coolingTemp - temp > minDifferenceTemp   //when temp > coolingTemp cools until coolingTemp - temp > minDifferenceTemp
*/
const int minCycles = 10; 
const float minDifferenceTemp = 4.0; // degres C

bool debugMode = false; //Turn output

bool P_TERMINATE = false;
bool fan_status = false;
int count = 0;
bool startCount = false;
//Prototype
void terminate_program(int sig);
void fanOn();
void fanOff();

int main(int argc,char* argv[])
{
    //Check mode
    if(controlMode>3||controlMode<1)
        return -1;
    //Proces args
    for(int i=0;i<argc;i++)
    {
        if(i>0&& argv[i]=="-d")
            debugMode = true;
    }
    
	//Init
    signal(SIGINT,terminate_program); //Interupt handle
    #ifdef RPI
        wiringPiSetup();
        pinMode(fan_control_pin,OUTPUT);
        digitalWrite(fan_control_pin,LOW);
    #endif
    //Core stage
    FILE *file;
	double T;
    float temp;
	float lastTemp = 0;
    if(debugMode)
        cout << "Start monitoring temperature of CPU" << endl;
	while(!P_TERMINATE)
    {
        //Open file
        file = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
        if (file == NULL)
            continue;
        
        //Process
        T = 0;
        temp = 0;    
        fscanf (file, "%lf", &T);
        temp = ((float)T)/1000;
        
        if(debugMode&&lastTemp!=temp) //DEBUG ->
        {    
            printf ("The temperature is %6.3f C.\n", temp);
            lastTemp = temp;
        }
            
        //Close opened file
        fclose (file);
        
        
        if((controlMode==1&&temp > coolingTemp)||(controlMode==2&&temp>coolingTemp)||(controlMode==3&&temp>coolingTemp))
        {   
            //Cooling needed
            
            if((count<=0&&controlMode==2)||(controlMode!=2))
            {    
                fanOn();
                if(controlMode==2)
                    startCount = true;
            }
            if(startCount&&count>0)
                count = 0;
        }
        else 
        {
            //Stop cooling
            
            if(controlMode==2)
            {
                if(startCount)
                    count ++;
                if(count >= minCycles)
                {
                    count = 0;
                    fanOff();
                    startCount = false;
                }
            }
            else if(controlMode==3)
            {
                if(coolingTemp - temp > minDifferenceTemp)
                {
                        fanOff();
                }
            }
            else if(controlMode == 1)
            {
                if(temp < coolingTemp)
                {
                    fanOff();
                }
                
            }
        }
            
        if(!P_TERMINATE) //Dont sleep if interrupted
            delay(sleepTime);
        
    }
	#ifdef RPI
        digitalWrite(fan_control_pin,LOW);
    #endif
    if(debugMode)
        cout << "Monitoring stoped" << endl;

	return 0;
}


void terminate_program(int sig)
{
	P_TERMINATE = true;
}

void fanOn()
{
    if(fan_status==false) //Turn on fan
    {
        #ifdef RPI
            digitalWrite(fan_control_pin,HIGH);
        #endif
        if(debugMode)
            cout << "On" << endl;
        fan_status = true;
    }
}
void fanOff()
{
    if(fan_status==true) //Turn off FAN
    {
        #ifdef RPI
            digitalWrite(fan_control_pin,LOW);
        #endif
        if(debugMode)
            cout << "Off" << endl;
        fan_status = false;
    }
}