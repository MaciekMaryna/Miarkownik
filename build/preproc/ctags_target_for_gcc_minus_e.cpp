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
# 44 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*

 *  Macros

 *================================================*/
# 47 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
// TODO find better name instead ...Metric
# 60 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*

 *  Enums

 *================================================*/
# 63 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
enum SystemError_t
{
    NO_ERROR,
    NOT_INITIALIZED,
    GENERAL_TEMP_SENSORS_ERROR,
    SPECIFIC_TEMP_SENSORS_ERROR,
    MEASUREMENT_RESULT_NOT_READY,
    SERVO_POSITIONING_ERROR,
    NUMBER_OF_ERRORS
};

/*================================================*

 * Structures and type defs

 *================================================*/
# 77 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
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
    uint32_t StartTaskSysTick;
    uint32_t EndTaskSysTick;
    uint32_t CounterTaskRuns;
    uint32_t CallPeriod;
} Task_t;

/*================================================*

 * Global variables

 *================================================*/
# 100 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Data_t SystemState;

// Servo driver PCA9685
Adafruit_PWMServoDriver myPwmDriver = Adafruit_PWMServoDriver();

const byte MyTempSensorsAddress[6][8] 
# 108 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                                          __attribute__((__progmem__)) 
# 108 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                                                  = {
    0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B /* Actual temp sensor!!!*/, 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26, 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F, 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3, 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F, 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1};

// 1-Wire object
OneWire onewire(2);
// DS18B20 MyTempSensors object
DS18B20 MyTempSensors(&onewire);

/*================================================*

 * RTOS task initialization

 *================================================*/
# 119 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
Task_t ReadTempSensorsRoutineMetric = {0, 0, 0, 1000 /* 1000*/}; // TODO align words order in arguments of _InitTask
Task_t CalculationRoutineMetric = {0, 0, 0, 1000};
Task_t ServoPositioningRoutineMetric = {0, 0, 0, 1000 /* 1000*/};
Task_t SendDataRoutineMetric = {0, 0, 0, 1000 /* target pertiod = 60000 (60s)*/};
Task_t BenchmarkRoutineMetric = {0, 0, 0, 10000000};

/*================================================*

 * Arduino config function

 *================================================*/
# 128 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
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
# 155 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SysTickIrq()
{
    
# 157 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("cli" ::: "memory")
# 157 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                 ;
    SysTick++;
    
# 159 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("sei" ::: "memory")
# 159 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
               ;
}

/*================================================*

 * Functions

 *================================================*/
# 165 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void ErrorStatusChange(Data_t *SystemState, SystemError_t IncomingError)
{
    // TODO change to macro
    if (SystemState->Error != IncomingError)
    {
        SystemState->PreviewError = SystemState->Error;
        SystemState->Error = (uint8_t)IncomingError;
    }
}

SystemError_t InitHwSystem(void)
{
    SystemError_t ReturnValue = NOT_INITIALIZED;
    SystemState.Error = (uint8_t)ReturnValue;
    SystemState.TempNominal = 55;
    SystemState.ServoPosition = 150;

    if (false == MyTempSensors.available())
    {
        ReturnValue = GENERAL_TEMP_SENSORS_ERROR;
    }
    else
    {
        ReturnValue = NO_ERROR;
    }
    ErrorStatusChange(&SystemState, ReturnValue);
    return ReturnValue;
}

/*================================================*

 * RTOS Functions

 *================================================*/
# 197 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void ReadTempSensorsRoutine(void)
{
    if (MyTempSensors.available())
    {

        if (true == MyTempSensors.request())
        {
            SystemState.TempActual = (uint8_t)MyTempSensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( MyTempSensorsAddress[0] ) ));
            for (byte i = 1; i < 6; i++) // counting starting from value 1 cause 0 value is actual temp sensor (see line below)
            {
                SystemState.Temp[i - 1] = (uint8_t)MyTempSensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( MyTempSensorsAddress[i] ) ));
            }
            SystemState.TimeStamp = SysTick;
        }
        else
        {
            ErrorStatusChange(&SystemState, MEASUREMENT_RESULT_NOT_READY);
        }
    }
}

void CalculationRoutine(void)
{
    if (SystemState.TempActual < 21)
    {
        SystemState.TempActual = 21;
    }
    else if (SystemState.TempActual > 36)
    {
        SystemState.TempActual = 36;
    }
    else
    {
        SystemState.ServoPosition = (SystemState.TempActual - 21) * ((100) / ((36) - (21))) /*!!! care about non-zero demoninator !!!*/; // value as % of full range (normalized to 100%)
        // TODO check osc feq in Adafruit library
    }
}

void ServoPositioningRoutine(void)
{
    if (true == myPwmDriver.setPWM(0 /* channel of PWM driver 0..15*/, 0, SystemState.ServoPosition * (((600) - (150)) / (100)) + 150))
    {
        // do nothing more than setPWM in line above
    }
    else
    {
        ErrorStatusChange(&SystemState, SERVO_POSITIONING_ERROR);
    }
    // TODO check phisical MIN and MAX position of servo
}

void SendDataRoutine(void)
{
    uint8_t *pSystemStateBytes = &SystemState.TempNominal; // pointer to first position of struct which is one(!) byte width
    String buf1;
    for (int i = 0; i < sizeof(Data_t); i++)
    {
        buf1 = String(buf1 + pSystemStateBytes[i] + ", ");
    }
    Serial.println(buf1);
}

void BenchmarkRoutine(void)
{
    Serial.println("ReadTempSensorsRoutine:");
    Serial.println("Runs: ");
    Serial.println("Time: ");
}

/*================================================*

 * Main routine

 *================================================*/
# 270 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void loop()
{
    InitHwSystem();

    MyTempSensors.request(); // Perhaps line to cut because it fitst request was made in setup() - check after temp sendning will work properly
    while (1)
    {
        if (SysTick >= ReadTempSensorsRoutineMetric.StartTaskSysTick + ReadTempSensorsRoutineMetric.CallPeriod) { ReadTempSensorsRoutineMetric.StartTaskSysTick = SysTick; (ReadTempSensorsRoutine)(); ReadTempSensorsRoutineMetric.CounterTaskRuns++; ReadTempSensorsRoutineMetric.EndTaskSysTick = SysTick; };
        if (SysTick >= CalculationRoutineMetric.StartTaskSysTick + CalculationRoutineMetric.CallPeriod) { CalculationRoutineMetric.StartTaskSysTick = SysTick; (CalculationRoutine)(); CalculationRoutineMetric.CounterTaskRuns++; CalculationRoutineMetric.EndTaskSysTick = SysTick; };
        if (SysTick >= ServoPositioningRoutineMetric.StartTaskSysTick + ServoPositioningRoutineMetric.CallPeriod) { ServoPositioningRoutineMetric.StartTaskSysTick = SysTick; (ServoPositioningRoutine)(); ServoPositioningRoutineMetric.CounterTaskRuns++; ServoPositioningRoutineMetric.EndTaskSysTick = SysTick; };
        if (SysTick >= SendDataRoutineMetric.StartTaskSysTick + SendDataRoutineMetric.CallPeriod) { SendDataRoutineMetric.StartTaskSysTick = SysTick; (SendDataRoutine)(); SendDataRoutineMetric.CounterTaskRuns++; SendDataRoutineMetric.EndTaskSysTick = SysTick; };
        if (SysTick >= BenchmarkRoutineMetric.StartTaskSysTick + BenchmarkRoutineMetric.CallPeriod) { BenchmarkRoutineMetric.StartTaskSysTick = SysTick; (BenchmarkRoutine)(); BenchmarkRoutineMetric.CounterTaskRuns++; BenchmarkRoutineMetric.EndTaskSysTick = SysTick; };
    }
}
