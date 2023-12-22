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

#define SERVO_CHANNEL 0 // channel of PWM driver 0..15
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
// TODO find better name instead ...Metric
#define _InitTask(TASK_NAME, CALL_PERIOD) \
    Task_t TASK_NAME##Metric = {(TASK_NAME), 0, 0, 0, CALL_PERIOD}

#define _RunTask(TASK_NAME)                                                           \
    if (SysTick >= TASK_NAME##Metric.StartTaskSysTick + TASK_NAME##Metric.CallPeriod) \
    {                                                                                 \
        TASK_NAME##Metric.StartTaskSysTick = SysTick;                                 \
        (TASK_NAME)();                                                                \
        TASK_NAME##Metric.CounterTaskRuns++;                                          \
        TASK_NAME##Metric.EndTaskSysTick = SysTick;                                   \
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

    uint8_t PreviewError;
    uint8_t Error;
} Data_t;

typedef struct
{
    void (*pFunction)(void);
    uint32_t StartTaskSysTick;
    uint32_t EndTaskSysTick;
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
 * RTOS task initialization
 *================================================*/
#line 120 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void setup();
#line 147 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SysTickIrq();
#line 157 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void ErrorStatusChange(Data_t *SystemState, SystemError_t IncomingError);
#line 167 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
SystemError_t InitHwSystem(Data_t *SystemState);
#line 189 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void ReadTempSensorsRoutine(void);
#line 202 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void CalculationRoutine(void);
#line 210 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void PwmRoutine(void);
#line 215 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void SendDataRoutine(void);
#line 232 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
void loop();
#line 112 "C:\\Users\\maryn\\OneDrive\\Pulpit\\Miarkownik\\Miarkownik.ino"
_InitTask(ReadTempSensorsRoutine, TEMP_READ_PERIOD); // TODO align words order in arguments of _InitTask
_InitTask(CalculationRoutine, CALCULATION_PERIOD);
_InitTask(PwmRoutine, SERVO_UPDATE_PERIOD);
_InitTask(SendDataRoutine, DATA_SEND_PERIOD);

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
 * Interrrupt Routines
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
void ErrorStatusChange(Data_t *SystemState, SystemError_t IncomingError)
{
    // TODO change to macro
    if (SystemState->Error != IncomingError)
    {
        SystemState->PreviewError = SystemState->Error;
        SystemState->Error = (uint8_t)IncomingError;
    }
}

SystemError_t InitHwSystem(Data_t *SystemState)
{
    SystemError_t ReturnValue = NOT_INITIALIZED;
    SystemState->Error = (uint8_t)ReturnValue;
    SystemState->TempNominal = 55;
    SystemState->ServoPosition = SERVO_MIN;

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
void ReadTempSensorsRoutine(void)
{
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

void CalculationRoutine(void)
{
    SystemState.ServoPosition = SERVO_MIN + (SystemState.TempActual - 22) * 10;
    // TODO ServoPosition is to small(8bit). Values of calculation are above 255!
    // TODO check osc feq in Adafruit library
    // Check SDA i SCL pin number
}

void PwmRoutine(void)
{
    myPwmDriver.setPWM(SERVO_CHANNEL, 0, SystemState.ServoPosition);
}

void SendDataRoutine(void)
{
    uint8_t CRC = 0xFF;                                    // TODO decision if calculation is neaded or remove CRC at all
    uint8_t *pSystemStateBytes = &SystemState.TempNominal; // pointer to first position of struct which is one(!) byte width

    for (int i = 0; i < sizeof(Data_t); i++)
    {
        sprintf(pSystemStateBytes, "%d, ", pSystemStateBytes[i]);
    }
    Serial.print(*pSystemStateBytes);
    Serial.println(CRC);
}

/*================================================*
 * Main routine
 *================================================*/

void loop()
{
    InitHwSystem(&SystemState);

    MyTempSensors.request(); // Perhaps line to cut because it fitst request was made in setup() - check after temp sendning will work properly
    while (1)
    {
        _RunTask(ReadTempSensorsRoutine);
        _RunTask(CalculationRoutine);
        _RunTask(PwmRoutine);
        _RunTask(SendDataRoutine);
    }
}
