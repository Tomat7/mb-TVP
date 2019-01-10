#define PLC_ID 0x0C
#define SKETCHFILE __FILE__ " "
#define SKETCHTIME __DATE__ " " __TIME__
#define IP_NETWORK 192, 168, 1, 0
#define IP_ADDR_BASE IP_NETWORK + 30
#define ETHERNET_IP IP_ADDR_BASE + PLC_ID  // задаём IP адрес Ethernet модуля 

#define ADC_RATE 200  // количество отсчетов АЦП на ПОЛУволну
#define WAVES 4       // количество обсчитываемых ПОЛУволн
#define I_ZERO 0 //1907
#define U_ZERO 0 //2113
#define I_RATIO 0.1
#define U_RATIO 0.1

#ifdef ESP32
#include <WiFi.h>
#include <ModbusIP_ESP8266.h>
#define WIFI_NAME "Tomat1"
#define WIFI_PASS "filimon7"
IPAddress local_IP(ETHERNET_IP);
IPAddress gateway(IP_NETWORK + 254);
IPAddress subnet(255, 255, 255, 0);
#define MAC1 0x32               // бесполезно :-)
#define MAC2 0x00
#endif

#include "Modbus.h"
ModbusIP mb;

#include "ASOLED.h"
#if defined(OLED_SH1106)
#define LCDX1 0           // смещение 1-го "столбца" на экране
#define LCDX2 65          // смещение 2-го "столбца" на экране
#else
#define LCDX1 1           // смещение 1-го "столбца" на экране
#define LCDX2 67          // смещение 2-го "столбца" на экране
#endif

#define hrSECONDS 0
#define hrI 1
#define hrU 2
#define hrP 3

#define Ipin 36
#define Upin 39

//int sensorPin = 34;    // select the input pin for the potentiometer
//int ledPin = 13; //LED_BUILTIN; // 13;      // select the pin for the LED

hw_timer_t * timer = NULL;
hw_timer_t * ADCtimer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
volatile SemaphoreHandle_t ADCSemaphore[3];
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE adcMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

volatile uint64_t SummI = 0;
volatile uint64_t SummU = 0;
volatile uint16_t ADCcounter = 0;
float I, U;

uint32_t msMicros;
uint32_t msReadTime, msTimerCh;
uint32_t ReadCycles = 0;
uint16_t ADCcounts;
uint16_t adcRate;
uint32_t isrCount = 0;

void setup()
{
  // declare the ledPin as an OUTPUT:
  Serial.begin(115200);
  ADCcounts = ADC_RATE * WAVES;
  adcRate = (uint16_t)(10000 / ADC_RATE);
  delay(300);
  Serial.print("I zero: ");
  Serial.println(ShiftTest(Ipin, 4096));
  delay(300);
  Serial.print("U zero: ");
  Serial.println(ShiftTest(Upin, 4096));

  setupADC();
  //startADC();
  setupTimer();
  setupADCTimer();

  LD.init();
  LD.clearDisplay();

  String strVersion = SKETCHFILE;
  String strVer = strVersion.substring(1 + strVersion.lastIndexOf('\\'));
  char chVer[16];
  strVer.toCharArray(chVer, 16);
  LD.printString_6x8(chVer, LCDX1, 0);

  delay(300);
  Serial.print(strVersion);
  Serial.println(F(SKETCHTIME));
  Serial.println(ADCcounts);
  Serial.println(adcRate);

  setupNetMB();

  mb.addHreg(hrSECONDS);
  mb.addHreg(hrI);
  mb.addHreg(hrU);
  mb.addHreg(hrP);
  delay(3000);
  msTimerCh = millis();
}

void loop()
{
  RMSintr();

  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
  {
    uint32_t isrTime = 0;
    // Read the interrupt count and time
    portENTER_CRITICAL(&timerMux);
    isrCount = isrCounter;
    isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerMux);

    mb.task();
    uint16_t secUptime = (uint16_t)(millis() / 1000);
    mb.Hreg(hrSECONDS, (uint16_t)isrCount);

    uint32_t msMillis = micros();
    //uint32_t msRead = micros() - msMillis;

    LD.clearDisplay();

    //msMillis = micros();
    Serial.println(msReadTime);

    mb.Hreg(hrI, round(I * 1000));
    Serial.print("CURRENT: ");
    Serial.println(I);
    mb.Hreg(hrU, round(U));
    Serial.print("VOLTAGE: ");
    Serial.println(U);

    uint16_t P = int(I * U);
    mb.Hreg(hrP, P);
    Serial.print("POWER: ");
    Serial.println(P);

    msMillis = micros();

    char readBuf[] = {"       "};
    LD.printString_6x8("read", 0, 0);
    dtostrf(msReadTime, 7, 0, readBuf);
    LD.printString_6x8(readBuf, 30, 0);
    dtostrf(ReadCycles, 7, 0, readBuf);
    LD.printString_6x8(readBuf, 74, 0);

    char iBuf[] = {"       "};
    dtostrf(I, 7, 3, iBuf);
    LD.printString_6x8("I:", 0, 2);
    LD.printString_6x8(iBuf, 12, 2);
    char uBuf[] = {"     "};
    dtostrf(U, 5, 0, uBuf);
    LD.printString_6x8("U:", 60, 2);
    LD.printString_6x8(uBuf, 70, 2);

    char powerBuf[] = {"     "};
    dtostrf(P, 5, 0, powerBuf);
    LD.printString_6x8("P:", 0, 5);
    LD.printString_6x8(powerBuf, 42, 5);

    char dispBuf[] = {"     "};
    uint32_t msDisp = micros() - msMillis;
    dtostrf(msDisp, 5, 0, dispBuf);
    LD.printString_6x8(dispBuf, 0, 7);

    msDisp = millis() - msTimerCh;
    dtostrf(msDisp, 5, 0, dispBuf);
    LD.printString_6x8(dispBuf, 34, 7);

    dtostrf(isrCount, 7, 0, readBuf);
    LD.printString_6x8(readBuf, 74, 7);

    ReadCycles = 0;

    msTimerCh = millis();
    if (isrCount > 70) timerWrite(timer, 0);
  }
  //delay(1000);

  if (isrCount == 41) {
    delay(5000);
    timerWrite(timer, 200000);
    timerAlarmEnable(timer);
    isrCount = 42;
  }
  if (isrCount == 45) delay(5000);

}
