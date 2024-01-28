# 1 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*
 * Includes
 *================================================*/
# 5 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 6 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 7 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2

// servo libs
# 10 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2
# 11 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 2

/*================================================*
 * Defines
 *================================================*/
# 27 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
// 100 for 9g blue servo //150
// 80 for TD-6620MG servo


// 680 for 9g blue servo //600
// 700 for TD-6620MG servo
# 51 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*
 *  Macros
 *================================================*/
# 63 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
/*================================================*
 *  Enums
 *================================================*/
enum SystemError_t
{
    NO_ERROR,
    NOT_INITIALIZED,
    GENERAL_TEMP_SENSORS_ERROR,
    SPECIFIC_TEMP_SENSORS_ERROR,
    MEASUREMENT_TEMP_SENSORS_NOT_READY,
    SERVO_POSITIONING_ERROR,
    NUMBER_OF_ERRORS
};

/*================================================*
 * Structures and type defs
 *================================================*/
typedef struct
{
    uint8_t TempNominal; // first position should have one byte width
    uint8_t TempActual;
    uint8_t Temp[6 - 1];
    uint8_t ServoPosition;
    uint32_t TimeStamp;

    uint8_t PreviousError;
    uint8_t Error;
} Data_t;

typedef struct
{
    void (*TaskName)(void);
    uint32_t StartTaskSysTick;
    uint32_t EndTaskSysTick;
    // TODO add acummulative time
    uint32_t CounterTaskRuns;
    uint32_t CallPeriod;
} Task_t;

/*================================================*
 * Global variables
 *================================================*/
uint32_t SysTick = 0;
uint32_t LastSysTick = 0;

Data_t SystemState;

// Servo driver PCA9685
Adafruit_PWMServoDriver myPwmDriver = Adafruit_PWMServoDriver();

const byte MyTempSensorsAddress[6][8] 
# 113 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
                                                          __attribute__((__progmem__)) 
# 113 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                                                                  = {
    0x28, 0xA0, 0x7D, 0x81, 0x4D, 0x20, 0x1, 0x2B /* Actual temp sensor!!!*/, 0x28, 0x50, 0xD2, 0x9B, 0x4D, 0x20, 0x1, 0x26, 0x28, 0x6E, 0xCA, 0x86, 0x4D, 0x20, 0x1, 0x3F, 0x28, 0xB7, 0xC, 0xA6, 0x4D, 0x20, 0x1, 0xA3, 0x28, 0xFF, 0x4D, 0xE4, 0xC1, 0x17, 0x5, 0x4F, 0x28, 0xFF, 0x6B, 0xCB, 0x83, 0x17, 0x4, 0xE1};

// 1-Wire object
OneWire onewire(2);
// DS18B20 MyTempSensors object
DS18B20 MyTempSensors(&onewire);

/*================================================*
 * Arduino config function
 *================================================*/
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
 * Interrrupt _Routines
 *================================================*/
void SysTickIrq()
{
    
# 153 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("cli" ::: "memory")
# 153 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
                 ;
    SysTick++;
    
# 155 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino" 3
   __asm__ __volatile__ ("sei" ::: "memory")
# 155 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
               ;
}

/*================================================*
 * Functions
 *================================================*/
Task_t *_InitTask(void (*preset_TaskName)(void), uint32_t preset_StartTaskSysTick, uint32_t preset_EndTaskSysTick, uint32_t preset_CounterTaskRuns, uint32_t preset_CallPeriod)
{
    Task_t *pTaskMetric = (Task_t *)malloc(sizeof(Task_t));

    pTaskMetric->TaskName = preset_TaskName;
    pTaskMetric->StartTaskSysTick = preset_StartTaskSysTick;
    pTaskMetric->EndTaskSysTick = preset_EndTaskSysTick;
    pTaskMetric->CounterTaskRuns = preset_CounterTaskRuns;
    pTaskMetric->CallPeriod = preset_CallPeriod;

    return pTaskMetric;
}

void ErrorStatusChange(Data_t *SystemState, SystemError_t IncomingError)
{
    // TODO change to macro
    if (SystemState->Error != IncomingError)
    {
        SystemState->PreviousError = SystemState->Error;
        SystemState->Error = (uint8_t)IncomingError;
    }
}

SystemError_t InitHwSystem(void)
{
    SystemError_t ReturnValue = NOT_INITIALIZED;
    SystemState.Error = (uint8_t)ReturnValue;
    SystemState.TempNominal = 55;
    SystemState.ServoPosition = 80;

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
 * RTOS Routines
 *================================================*/
void ReadTempSensors_Routine(void)
{
    if (MyTempSensors.available())
    {

        if (true == MyTempSensors.request())
        {
            SystemState.TempActual = (uint8_t)MyTempSensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( MyTempSensorsAddress[0] ) ));
            for (byte i = 1; i < 6; i++) // counting starts from value 1 cause 0 value is actual temp sensor (see line below)
            {
                SystemState.Temp[i - 1] = (uint8_t)MyTempSensors.readTemperature(( reinterpret_cast< const __FlashStringHelper * >( MyTempSensorsAddress[i] ) ));
            }
            SystemState.TimeStamp = SysTick;
        }
        else
        {
            ErrorStatusChange(&SystemState, MEASUREMENT_TEMP_SENSORS_NOT_READY);
        }
    }
}

void Calculation_Routine(void)
{
    if (SystemState.TempActual < 21)
    {
        SystemState.TempActual = 21;
    }
    else if (SystemState.TempActual > 71 /* 36*/)
    {
        SystemState.TempActual = 71 /* 36*/;
    }
    else
    {
        SystemState.ServoPosition = (SystemState.TempActual - 21) * ((100) / ((71 /* 36*/) - (21))) /*!!! care about non-zero demoninator !!!*/; // value as % of full range (normalized to 100%)
    }
}

void ServoPositioning_Routine(void)
{
    static uint8_t PreviousServoPosition;

    // PreviousServoPosition = (PreviousServoPosition == 250) ? 280 : 250;
    // myPwmDriver.setPWM(SERVO_CHANNEL, 0, PreviousServoPosition);
    if (false != myPwmDriver.setPWM(0 /* channel of PWM driver 0..15*/, 0, SystemState.ServoPosition * (((700) - (80)) / (100)) + 80))
    {
        ErrorStatusChange(&SystemState, SERVO_POSITIONING_ERROR);
    }

    if (PreviousServoPosition > SystemState.ServoPosition) // air intake decrease command
    {
        // TODO
        // double beep
        // double led flash
    }

    if (PreviousServoPosition < SystemState.ServoPosition) // air intake increase command
    {
        // single beep
        // single led flash
    }

    PreviousServoPosition = SystemState.ServoPosition;
}

void SendData_Routine(void)
{
    uint8_t *pSystemStateBytes = &SystemState.TempNominal; // pointer to first position of struct which is one(!) byte width
    String buf1;
    for (int i = 0; i < sizeof(Data_t); i++)
    {
        buf1 = String(buf1 + pSystemStateBytes[i] + ", ");
    }
    Serial.println(buf1);
}

void Benchmark_Routine(void)
{
    Serial.println("ReadTempSensors_Routine:");
    Serial.println("Runs: ");
    Serial.println("Time: ");
}

/*================================================*
 * Main routine
 *================================================*/

void loop()
{
    InitHwSystem();

    Task_t *ReadTempSensors_Handler = _InitTask(ReadTempSensors_Routine, 0, 0, 0, 1000 /* 1000*/);
    Task_t *Calculation_Handler = _InitTask(Calculation_Routine, 0, 0, 0, 1000);
    Task_t *ServoPositioning_Handler = _InitTask(ServoPositioning_Routine, 0, 0, 0, 1000 /* 1000*/);
    Task_t *SendData_Handler = _InitTask(SendData_Routine, 0, 0, 0, 1000 /* target pertiod = 60000 (60s)*/);
    Task_t *Benchmark_Handler = _InitTask(Benchmark_Routine, 0, 0, 0, 10000000);

    while (1)
    {
        tone(3, 1000, 500);
        if (SysTick >= (ReadTempSensors_Handler->StartTaskSysTick + ReadTempSensors_Handler->CallPeriod)) { ReadTempSensors_Handler->StartTaskSysTick = SysTick; (ReadTempSensors_Routine)(); ReadTempSensors_Handler->CounterTaskRuns++; ReadTempSensors_Handler->EndTaskSysTick = SysTick; };
        if (SysTick >= (Calculation_Handler->StartTaskSysTick + Calculation_Handler->CallPeriod)) { Calculation_Handler->StartTaskSysTick = SysTick; (Calculation_Routine)(); Calculation_Handler->CounterTaskRuns++; Calculation_Handler->EndTaskSysTick = SysTick; };
        if (SysTick >= (ServoPositioning_Handler->StartTaskSysTick + ServoPositioning_Handler->CallPeriod)) { ServoPositioning_Handler->StartTaskSysTick = SysTick; (ServoPositioning_Routine)(); ServoPositioning_Handler->CounterTaskRuns++; ServoPositioning_Handler->EndTaskSysTick = SysTick; };
        if (SysTick >= (SendData_Handler->StartTaskSysTick + SendData_Handler->CallPeriod)) { SendData_Handler->StartTaskSysTick = SysTick; (SendData_Routine)(); SendData_Handler->CounterTaskRuns++; SendData_Handler->EndTaskSysTick = SysTick; };

        //_RunTask(Benchmark);
    }
}

/*================================================*
 * TODO
 *================================================*
    Decision to split regulation and monitoring functionality to separate microcontrolers
    Benchmark task

    SD card recording
    LCD 4 lines or 2 lines + keys
    RGB strip vizualisation
    Buzzer
    LEDs shows +/- move of servo

    Control panel options:
    ==============================
    Switches and buttons
        auto - PID regulation
        manu - MIN position
        manu - MAX position

        manu - increese ait intake
        manu - decrease air intake

    Status lamps
        LED - regulation trend (buzzer)
        RGB LED STRIP - buffer

    Displays
        LCD
         - actual temp
         - nominal temp,
         - temps of 4 buffer points

    Other hardware features
        - RTC
        - micro SD card logging with time TimeStamp
        - Front panel LEDs show: openinig and closeing, min and max pos ???

    Further features:
        - voltometer of UPS battery
        - Pure sineDC/Ac converter
        - UPS how to swich power without glitch
        - Supply all devices of system from safet power source

 *================================================*/
