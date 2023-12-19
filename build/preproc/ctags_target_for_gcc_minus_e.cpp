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
# 41 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*

 *  Enums

 *================================================*/
# 44 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
enum SystemError_t
{
    NO_ERROR,
    NOT_INITIALIZED,
    GENERAL_TEMP_SENSORS_ERROR,
    SPECIFIC_TEMP_SENSORS_ERROR,
    NUMBER_OF_ERRORS
};

/*================================================*

 * Structures and type defs

 *================================================*/
# 56 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
typedef struct
{
    uint8_t TempNominal; // first position should have one byte width
    uint8_t TempActual;
    uint8_t Temp[6 - 1];
    uint8_t ServoPosition;
    uint32_t TimeStamp;

    uint8_t PreviewError;
    uint8_t Error;
} Data_t;

typedef struct
{
    void (*pFunction)(void);
    uint32_t StartTaskSysTick;
    uint32_t EndTaskSysTick;
    uint32_t TaskRunsCounter;
    uint32_t CallPeriod; // TODO perhaps magic numbers to set the calling periods will be not needed any more
} Task_t;

/*================================================*

 * Global variables

 *================================================*/
# 80 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Data_t SystemState;

// Servo driver PCA9685
Adafruit_PWMServoDriver myPwmDriver = Adafruit_PWMServoDriver();

const byte MyTempSensorsAddress[6][8] 
# 88 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                                          __attribute__((__progmem__)) 
# 88 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                                                  = {
    0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B /* Actual temp sensor!!!*/, 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26, 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F, 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3, 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F, 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1};

// 1-Wire object
OneWire onewire(2);
// DS18B20 MyTempSensors object
DS18B20 MyTempSensors(&onewire);

/*================================================*

 * RTOS Global variables

 *================================================*/
# 100 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
// set metric for all of the tasks which running prediodicaly win main loop
Task_t *PwmRoutineMetric; // TODO find better name instead ...Metric

/*================================================*

 * Arduino config function

 *================================================*/
# 106 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup()
{

    /*Sys Timer Config*/
    Timer1.initialize(1000 /* microseconds*/);
    Timer1.attachInterrupt(SysTickIrq);

    /* GPIO Config */
    pinMode(13, 0x1);

    /*Serial Port Config*/
    Serial.begin(115200);

    /* DS18B20 MyTempSensors setup */
    MyTempSensors.begin();

    /* The first requests to all MyTempSensors for measurement */
    MyTempSensors.request();

    /* Servo config */
    myPwmDriver.begin();
    myPwmDriver.setPWMFreq(60);
}

/*================================================*

 * Interrrupt Routines

 *================================================*/
# 133 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SysTickIrq()
{
    
# 135 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("cli" ::: "memory")
# 135 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                 ;

    SysTick++;

    
# 139 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("sei" ::: "memory")
# 139 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
               ;
}

/*================================================*

 * Functions

 *================================================*/
# 145 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void ErrorStatusChange(Data_t *SystemState, SystemError_t IncomingError)
{
    // TODO change to macro
    if (SystemState->Error != IncomingError)
    {
        SystemState->PreviewError = SystemState->Error;
        SystemState->Error = (uint8_t)IncomingError;
    }
}

SystemError_t InitSystem(Data_t *SystemState)
{
    SystemError_t ReturnValue = NOT_INITIALIZED;
    SystemState->Error = (uint8_t)ReturnValue;
    SystemState->TempNominal = 55;
    SystemState->ServoPosition = 150;

    if (!MyTempSensors.available())
    {
        ReturnValue = GENERAL_TEMP_SENSORS_ERROR;
    }
    else
    {
        ReturnValue = NO_ERROR;
    }
    ErrorStatusChange(SystemState, ReturnValue);
    return ReturnValue;
}

/*================================================*

 * RTOS Functions

 *================================================*/
# 178 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void InitTask(Task_t *TaskMetric, uint32_t CallPrd)
{

    TaskMetric->StartTaskSysTick = 0;
    TaskMetric->EndTaskSysTick = 0;
    TaskMetric->TaskRunsCounter = 0;
    TaskMetric->CallPeriod = CallPrd;
}

void PwmRoutine(void)
{

    // if (SysTick >= PwmRoutineMetric->StartTaskSysTick + PwmRoutineMetric->CallPeriod)
    {
        PwmRoutineMetric->StartTaskSysTick = SysTick;
        myPwmDriver.setPWM(0 /* channel of PWM driver 0..15*/, 0, SystemState.ServoPosition);
        // Serial.print(SystemState.ServoPosition);
        // Serial.println("#");
    }
    PwmRoutineMetric->TaskRunsCounter++;
    PwmRoutineMetric->EndTaskSysTick = SysTick;
}

/*================================================*

 * Main routine

 *================================================*/
# 205 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void loop()
{
    // register all of the tasks which running prediodicaly win main loop
    InitTask(PwmRoutineMetric, 1000 /* 1000*/);

    InitSystem(&SystemState);

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
        // TODO check osc feq in Adafruit library
        // Check SDA i SCL pin number
        if (SysTick >= PwmRoutineMetric->StartTaskSysTick + PwmRoutineMetric->CallPeriod)
        {
            PwmRoutine();
        }

        if (SysTick >= LastSysTick + 1000 /* target pertiod = 60000 (60s)*/)
        {
            // LastSysTick = SysTick;
            uint8_t CRC = 0xFF; // TODO calculation if neaded
            uint8_t *pSystemStateBytes = &SystemState.TempNominal; // pointer to first position of struct which is one(!) byte width

            for (int i = 0; i < sizeof(Data_t); i++)
            {
                Serial.print(*pSystemStateBytes);
                Serial.print(", ");
                pSystemStateBytes++;
            }

            Serial.println(CRC);
        }
    }
}
