#include <Arduino.h>
#line 1 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/**************************************************
 *** Includes**************************************
 **************************************************/
// #include <TimerOne.h>
#include <Servo.h>
#include <OneWire.h>
#include <DS18B20.h>

/**************************************************
 *** Defines **************************************
 **************************************************/
#define SYS_TIC_PERIOD (100000) // microseconds
#define TICKS_PER_SEC (1000000 / SYS_TIC_PERIOD)
#define LED_PERIOD (300)

#define ON true
#define OFF false

#define SERVO_PIN (9)

#define ONEWIRE_PIN (2)

#define T1 0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B
#define T2 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26
#define T3 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F
#define T4 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3
#define T5 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F
#define T6 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1

#define NUMBER_OF_TEMPSENSORS (6)
/**************************************************
 *** Macros ***************************************
 **************************************************/

/**************************************************
 *** Global variables *****************************
 **************************************************/
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Servo myServo;

const byte sensorsAddress[NUMBER_OF_TEMPSENSORS][8] PROGMEM = {
    T1, T2, T3, T4, T5, T6};

// 1-Wire object
OneWire onewire(ONEWIRE_PIN);
// DS18B20 sensors object
DS18B20 sensors(&onewire);

/**************************************************
 *** Arduino config function **********************
 **************************************************/
#line 54 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup();
#line 80 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SysTickIrq();
#line 85 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
bool Led_Toggle(uint8_t pin);
#line 102 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void Show();
#line 112 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void loop();
#line 54 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup()
{
    /*Servo Config*/
    myServo.attach(SERVO_PIN);

    /*Sys Timer Config*/
    // doesn't work because using the same interrupt as Servo lib
    //  Timer1.initialize(SYS_TIC_PERIOD); // 1000 = 1ms
    //  Timer1.attachInterrupt(SysTickIrq);

    /* GPIO Config */
    pinMode(LED_BUILTIN, OUTPUT);

    /*Serial Port Config*/
    Serial.begin(115200);

    // DS18B20 sensors setup
    sensors.begin();

    // The first requests to all sensors for measurement
    sensors.request();
}

/**************************************************
 *** Functions ************************************
 **************************************************/
void SysTickIrq()
{
    SysTick++;
}

bool Led_Toggle(uint8_t pin)
{
    bool Led_State = digitalRead(pin);

    if (false == Led_State)
    {
        Led_State = true;
    }
    else
    {
        Led_State = false;
    }

    digitalWrite(pin, Led_State);
    return Led_State;
}

void Show()
{
    Serial.print("SysTck: ");
    Serial.print(SysTick);
}

/**************************************************
 *** Main routine**********************************
 **************************************************/

void loop()
{
    uint8_t Srv_MinimalPos = 0;
    uint8_t Srv_ActualPos;
    uint8_t Srv_NominalPos = 36;
    uint8_t NormalBodyTemp = 36;
    LastSysTick = SysTick;

    myServo.write(10);

    while (1)
    {

        Srv_ActualPos = Srv_NominalPos -
                        Srv_NominalPos * (uint8_t)sensors.readTemperature(FA(sensorsAddress[5])) / NormalBodyTemp;

        if (0 == SysTick % 100)
        {

            //  If the sesor measurement is ready, prnt the results
            if (sensors.available())
            {
                for (byte i = 0; i < NUMBER_OF_TEMPSENSORS; i++)
                {
                    // Reads the temperature from sensor
                    // *** Indexed address from Flash memory
                    float temperature = sensors.readTemperature(FA(sensorsAddress[i]));

                    // Prints the temperature on Serial Monitor
                    Serial.print(F("#"));
                    Serial.print(i + 1);
                    Serial.print(F(": "));
                    Serial.print(temperature);
                    Serial.print(F("'C   "));
                }
                Serial.print(F("SysTick: "));
                Serial.println(myServo.read());

                // Another requests to all sensors for measurement
                sensors.request();
            }
        }
        SysTick++;
    }
}

