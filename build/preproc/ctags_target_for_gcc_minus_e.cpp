# 1 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*

 * Includes

 *================================================*/
# 4 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
# 5 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 6 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 7 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2

// servo libs
# 10 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 11 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2

/*================================================*

 * Defines

 *================================================*/
# 38 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*

 *  Macros

 *================================================*/
# 42 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*

 * Structures and type defs

 *================================================*/
# 46 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
typedef struct Data
{
    uint32_t TimeStamp; // SysTick dump of measurement
    uint8_t TempNominal;
    uint8_t TempActual;
    uint8_t Temp[6 - 1];
    uint8_t ServoPosition;

} Data_t;

/*================================================*

 * Global variables

 *================================================*/
# 59 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Data_t SystemState;

// Servo driver PCA9685
Adafruit_PWMServoDriver myPwmDriver = Adafruit_PWMServoDriver();

const byte MyTempSensorsAddress[6][8] 
# 67 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                                          __attribute__((__progmem__)) 
# 67 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                                                  = {
    0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B /* Actual temp sensor!!!*/, 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26, 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F, 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3, 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F, 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1};

// 1-Wire object
OneWire onewire(2);
// DS18B20 MyTempSensors object
DS18B20 MyTempSensors(&onewire);

/*================================================*

 * Arduino config function

 *================================================*/
# 78 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup()
{

    /*Sys Timer Config*/
    Timer1.initialize(1000 /* microseconds*/);
    Timer1.attachInterrupt(SysTickIrq);

    /* GPIO Config */
    pinMode(13, 0x1);

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
# 105 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SysTickIrq()
{
    
# 107 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("cli" ::: "memory")
# 107 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                 ;

    SysTick++;

    
# 111 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("sei" ::: "memory")
# 111 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
               ;
}

/*================================================*

 * Main routine

 *================================================*/
# 118 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void loop()
{
    uint8_t Srv_MinimalPos = 0;
    uint8_t Srv_ActualPos;
    uint8_t Srv_NominalPos = 36;
    uint8_t NormalBodyTemp = 36;

    MyTempSensors.request();

    while (1)
    {

        if (SysTick >= LastSysTick + 1000 /* 1000*/)
        {
            LastSysTick = SysTick;
            SystemState.TimeStamp = LastSysTick;

            if (MyTempSensors.available())
            {
                SystemState.TempActual = (uint8_t)MyTempSensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( MyTempSensorsAddress[0] ) ));
                for (byte i = 1; i < 6; i++) // counting starting from value 1 cause 0 value is actual temp sensor (see line below)
                {
                    SystemState.Temp[i - 1] = (uint8_t)MyTempSensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( MyTempSensorsAddress[i] ) ));
                }
                MyTempSensors.request();
            }
        }

        // if (SysTick >= LastSysTick + CALCULATION_PERIOD)
        {
            // LastSysTick = SysTick;
            SystemState.ServoPosition = 150 + (SystemState.TempActual - 22) * 30;
        }

        // if (SysTick >= LastSysTick + SERVO_UPDATE_PERIOD)
        {
            // LastSysTick = SysTick;
            myPwmDriver.setPWM(0 /* channel of PWM driver 0..15*/, 0, SystemState.ServoPosition);
        }

        if (SysTick >= LastSysTick + 1000 /* target pertiod = 60000 (60s)*/)
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
