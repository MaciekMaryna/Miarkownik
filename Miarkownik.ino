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
#define SEND_DATA_PERIOD 1000          // target pertiod = 60000 (60s)
#define READ_TEMP_SENSORS_PERIOD 1000  // 1000
#define SERVO_POSITIONINIG_PERIOD 1000 // 1000
#define CALCULATION_PERIOD 1000
#define BENCHMARK_PERIOD 10000000

#define SERVO_CHANNEL 0 // channel of PWM driver 0..15
#define SERVO_MIN 80
// 100 for 9g blue servo //150
// 80 for TD-6620MG servo

#define SERVO_MAX 700
// 680 for 9g blue servo //600
// 700 for TD-6620MG servo
#define SERVO_RATIO (((SERVO_MAX) - (SERVO_MIN)) / (100))

#define TEMP_LOW_LIMIT 21
#define TEMP_HIGH_LIMIT 71                                          // 36
#define TEMP_RATIO ((100) / ((TEMP_HIGH_LIMIT) - (TEMP_LOW_LIMIT))) //!!! care about non-zero demoninator !!!

#define ONEWIRE_PIN 2
#define BUZZER_PIN 3

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
#define _RunTask(TASK_NAME)                                                                   \
    if (SysTick >= (TASK_NAME##_Handler->StartTaskSysTick + TASK_NAME##_Handler->CallPeriod)) \
    {                                                                                         \
        TASK_NAME##_Handler->StartTaskSysTick = SysTick;                                      \
        (TASK_NAME##_Routine)();                                                              \
        TASK_NAME##_Handler->CounterTaskRuns++;                                               \
        TASK_NAME##_Handler->EndTaskSysTick = SysTick;                                        \
    }

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
    uint8_t Temp[NUMBER_OF_TEMP_SENSORS - 1];
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

const byte MyTempSensorsAddress[NUMBER_OF_TEMP_SENSORS][8] PROGMEM = {
    T1, T2, T3, T4, T5, T6};

// 1-Wire object
OneWire onewire(ONEWIRE_PIN);
// DS18B20 MyTempSensors object
DS18B20 MyTempSensors(&onewire);

/*================================================*
 * Arduino config function
 *================================================*/
void setup()
{

    /*Sys Timer Config*/
    Timer1.initialize(SYS_TIC_PERIOD);
    Timer1.attachInterrupt(SysTickIrq);

    /* GPIO Config */
    pinMode(LED_BUILTIN, OUTPUT);

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
    noInterrupts();
    SysTick++;
    interrupts();
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
    SystemState.ServoPosition = SERVO_MIN;

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
            SystemState.TempActual = (uint8_t)MyTempSensors.readTemperature(FA(MyTempSensorsAddress[0]));
            for (byte i = 1; i < NUMBER_OF_TEMP_SENSORS; i++) // counting starts from value 1 cause 0 value is actual temp sensor (see line below)
            {
                SystemState.Temp[i - 1] = (uint8_t)MyTempSensors.readTemperature(FA(MyTempSensorsAddress[i]));
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
    if (SystemState.TempActual < TEMP_LOW_LIMIT)
    {
        SystemState.TempActual = TEMP_LOW_LIMIT;
    }
    else if (SystemState.TempActual > TEMP_HIGH_LIMIT)
    {
        SystemState.TempActual = TEMP_HIGH_LIMIT;
    }
    else
    {
        SystemState.ServoPosition = (SystemState.TempActual - TEMP_LOW_LIMIT) * TEMP_RATIO; // value as % of full range (normalized to 100%)
    }
}

void ServoPositioning_Routine(void)
{
    static uint8_t PreviousServoPosition;

    // PreviousServoPosition = (PreviousServoPosition == 250) ? 280 : 250;
    // myPwmDriver.setPWM(SERVO_CHANNEL, 0, PreviousServoPosition);
    if (false != myPwmDriver.setPWM(SERVO_CHANNEL, 0, SystemState.ServoPosition * SERVO_RATIO + SERVO_MIN))
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

    Task_t *ReadTempSensors_Handler = _InitTask(ReadTempSensors_Routine, 0, 0, 0, READ_TEMP_SENSORS_PERIOD);
    Task_t *Calculation_Handler = _InitTask(Calculation_Routine, 0, 0, 0, CALCULATION_PERIOD);
    Task_t *ServoPositioning_Handler = _InitTask(ServoPositioning_Routine, 0, 0, 0, SERVO_POSITIONINIG_PERIOD);
    Task_t *SendData_Handler = _InitTask(SendData_Routine, 0, 0, 0, SEND_DATA_PERIOD);
    Task_t *Benchmark_Handler = _InitTask(Benchmark_Routine, 0, 0, 0, BENCHMARK_PERIOD);

    while (1)
    {
        _RunTask(ReadTempSensors);
        _RunTask(Calculation);
        _RunTask(ServoPositioning);
        _RunTask(SendData);

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