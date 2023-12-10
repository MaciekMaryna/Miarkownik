# 1 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/**************************************************

 *** Includes**************************************

 **************************************************/
# 4 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
// #include <TimerOne.h>
# 6 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 7 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 8 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2

/**************************************************

 *** Defines **************************************

 **************************************************/
# 31 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/**************************************************

 *** Macros ***************************************

 **************************************************/
# 35 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/**************************************************

 *** Global variables *****************************

 **************************************************/
# 38 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Servo myServo;

const byte sensorsAddress[(6)][8] 
# 43 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                                   __attribute__((__progmem__)) 
# 43 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                                           = {
    0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B, 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26, 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F, 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3, 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F, 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1};

// 1-Wire object
OneWire onewire((2));
// DS18B20 sensors object
DS18B20 sensors(&onewire);

/**************************************************

 *** Arduino config function **********************

 **************************************************/
# 54 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup()
{
    /*Servo Config*/
    myServo.attach((9));

    /*Sys Timer Config*/
    // doesn't work because using the same interrupt as Servo lib
    //  Timer1.initialize(SYS_TIC_PERIOD); // 1000 = 1ms
    //  Timer1.attachInterrupt(SysTickIrq);

    /* GPIO Config */
    pinMode(13, 0x1);

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
# 80 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
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
# 112 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
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
                        Srv_NominalPos * (uint8_t)sensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( sensorsAddress[5] ) )) / NormalBodyTemp;

        if (0 == SysTick % 100)
        {

            //  If the sesor measurement is ready, prnt the results
            if (sensors.available())
            {
                for (byte i = 0; i < (6); i++)
                {
                    // Reads the temperature from sensor
                    // *** Indexed address from Flash memory
                    float temperature = sensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( sensorsAddress[i] ) ));

                    // Prints the temperature on Serial Monitor
                    Serial.print((reinterpret_cast<const __FlashStringHelper *>(
# 141 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 141 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                "#"
# 141 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                ); &__c[0];}))
# 141 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                )));
                    Serial.print(i + 1);
                    Serial.print((reinterpret_cast<const __FlashStringHelper *>(
# 143 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 143 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                ": "
# 143 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                ); &__c[0];}))
# 143 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                )));
                    Serial.print(temperature);
                    Serial.print((reinterpret_cast<const __FlashStringHelper *>(
# 145 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 145 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                "'C   "
# 145 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                ); &__c[0];}))
# 145 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                )));
                }
                Serial.print((reinterpret_cast<const __FlashStringHelper *>(
# 147 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                            (__extension__({static const char __c[] __attribute__((__progmem__)) = (
# 147 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                            "SysTick: "
# 147 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                            ); &__c[0];}))
# 147 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                            )));
                Serial.println(myServo.read());

                // Another requests to all sensors for measurement
                sensors.request();
            }
        }
        SysTick++;
    }
}
