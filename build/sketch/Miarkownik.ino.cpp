#include <Arduino.h>
#line 1 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*
 * Includes
 *================================================*/
#include <TimerOne.h>
#include <OneWire.h>
#include <DS18B20.h>

// servo libs
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

/*================================================*
 * Defines
 *================================================*/
#define SYS_TIC_PERIOD 1000                      // microseconds
#define TICKS_PER_SEC (1000000 / SYS_TIC_PERIOD) // freq of SysTck

#define LED_PERIOD 300
#define DATA_SEND_PERIOD 1000    // target pertiod = 60000 (60s)
#define TEMP_READ_PERIOD 1000    // 1000
#define SERVO_UPDATE_PERIOD 1000 // 1000
#define CALCULATION_PERIOD 1000

#define SERVO_NUM_0 0 // channel of PWM driver 0..15
#define SERVO_MIN 150
#define SERVO_MAX 600

#define ONEWIRE_PIN 2

#define T1 0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B // Actual temp sensor!!!
#define T2 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26
#define T3 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F
#define T4 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3
#define T5 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F
#define T6 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1

#define NUMBER_OF_TEMP_SENSORS 6
/*================================================*
 *  Macros
 *================================================*/

/*================================================*
 * Structures and type defs
 *================================================*/

typedef struct Data
{
    uint32_t TimeStamp; // SysTick dump of measurement
    uint8_t TempNominal;
    uint8_t TempActual;
    uint8_t Temp[NUMBER_OF_TEMP_SENSORS - 1];
    uint8_t ServoPosition;

} Data_t;

/*================================================*
 * Global variables
 *================================================*/
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Data_t SystemState;

// Servo driver PCA9685
Adafruit_PWMServoDriver myPwmDriver = Adafruit_PWMServoDriver();

const byte MyTempSensorsAddress[NUMBER_OF_TEMP_SENSORS][8] PROGMEM = {
    T1, T2, T3, T4, T5, T6};

// 1-Wire object
OneWire onewire(ONEWIRE_PIN);
// DS18B20 MyTempSensors object
DS18B20 MyTempSensors(&onewire);

/*================================================*
 * Arduino config function
 *================================================*/
#line 78 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup();
#line 105 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SysTickIrq();
#line 118 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void loop();
#line 78 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup()
{

    /*Sys Timer Config*/
    Timer1.initialize(SYS_TIC_PERIOD);
    Timer1.attachInterrupt(SysTickIrq);

    /* GPIO Config */
    pinMode(LED_BUILTIN, OUTPUT);

    /*Serial Port Config*/
    Serial.begin(115200);

    // DS18B20 MyTempSensors setup
    MyTempSensors.begin();

    // The first requests to all MyTempSensors for measurement
    MyTempSensors.request();

    // Servo config
    myPwmDriver.begin();
    myPwmDriver.setPWMFreq(60);
}

/*================================================*
 * Functions
 *================================================*/
void SysTickIrq()
{
    noInterrupts();

    SysTick++;

    interrupts();
}

/*================================================*
 * Main routine
 *================================================*/

void loop()
{
    uint8_t Srv_MinimalPos = 0;
    uint8_t Srv_ActualPos;
    uint8_t Srv_NominalPos = 36;
    uint8_t NormalBodyTemp = 36;

    MyTempSensors.request();

    while (1)
    {

        if (SysTick >= LastSysTick + TEMP_READ_PERIOD)
        {
            LastSysTick = SysTick;
            SystemState.TimeStamp = LastSysTick;

            if (MyTempSensors.available())
            {
                SystemState.TempActual = (uint8_t)MyTempSensors.readTemperature(FA(MyTempSensorsAddress[0]));
                for (byte i = 1; i < NUMBER_OF_TEMP_SENSORS; i++) // counting starting from value 1 cause 0 value is actual temp sensor (see line below)
                {
                    SystemState.Temp[i - 1] = (uint8_t)MyTempSensors.readTemperature(FA(MyTempSensorsAddress[i]));
                }
                MyTempSensors.request();
            }
        }

        // if (SysTick >= LastSysTick + CALCULATION_PERIOD)
        {
            // LastSysTick = SysTick;
            SystemState.ServoPosition = SERVO_MIN + (SystemState.TempActual - 22) * 30;
        }

        // if (SysTick >= LastSysTick + SERVO_UPDATE_PERIOD)
        {
            // LastSysTick = SysTick;
            myPwmDriver.setPWM(SERVO_NUM_0, 0, SystemState.ServoPosition);
        }

        if (SysTick >= LastSysTick + DATA_SEND_PERIOD)
        {
            // TODO serialization

            // LastSysTick = SysTick;
            Data_t *pSystemState = &SystemState;

            for (int i = 0; i < sizeof(SystemState), i++)
            {
                Serial.print(*pSystemState);
                pSystemState++
            }

            // Serial.print(SystemState.TimeStamp);
            // Serial.print(", ");

            // Serial.print(SystemState.TempNominal);
            // Serial.print(", ");

            // Serial.print(SystemState.TempActual);
            // Serial.print(", ");

            // for (byte i = 1; i < NUMBER_OF_TEMP_SENSORS; i++)
            // {
            //     Serial.print(SystemState.Temp[i - 1]);
            //     Serial.print(", ");
            // }

            // Serial.println(SystemState.ServoPosition);
        }
    }
}
