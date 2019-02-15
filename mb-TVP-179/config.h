#define SKETCHFILE __FILE__ " "
#define SKETCHTIME __DATE__ " " __TIME__
#define DS_CONVTIME 750     // как часто опрашивать DS18B20 (миллисекунды)
#define MB_TIMEOUT 20       // как долго можно работать/кликать_клапаном без мастера Модбаса (секунды)
#define SERIALSPEED 115200  // скорость в последовательном порту
#define SERIAL_CONFIG       // если нужно выдать только конфигурацию в Serial
#define SERIAL_INFO         // если нужно постоянно выдавать информацию в Serial
//#define SERIAL_DEBUG        // доп. информация в Serial и в регистры Модбаса ** нужно поправить библиотеки! **

#include "Modbus.h"
ModbusIP mb;

#include "DStemp.h"
DSThermometer ds18b20[] = DSPINS;

#include "HDvalve.h"
Valve valve[] = VALVEPINS;

#ifdef PRESSURE_BMP
#include "BMP280x.h"
BMP280x bmp280(BMP280_ADDRESS);
#endif

/*
#ifdef PRESSURE_MPX
#include "MPX5010x.h"
MPX5010x mpx5010dp(MPX5010_PIN);
#endif
*/
#ifdef PRESSURE_MPX
#include "ADCmulti.h"
ADCmulti mpx5010dp(MPX5010_PIN);
//int Pins[] = { MPX5010_PIN };
//int Vals[] = { &mpxRAW };
#endif

// ===
#include "ASOLED.h"
#if defined(OLED_SH1106)
#define LCDX1 0           // смещение 1-го "столбца" на экране
#define LCDX2 65          // смещение 2-го "столбца" на экране
#else
#define LCDX1 1           // смещение 1-го "столбца" на экране
#define LCDX2 67          // смещение 2-го "столбца" на экране
#endif

// ===
#define hrSECONDS 0                   // регистр-счетчик секунд uptime'а
#define hrTEMP hrSECONDS + 1          // первый регистр с температурой
#define hrPRESSURE hrTEMP + nSensor   // регистр давления
#define hrOPEN hrPRESSURE + 1         // первый регистр с данными для/от клапанов
#define hrCLOSE hrOPEN + 1            // "базовая" скорость - объем собранный за 1000 кликов
#define hrCLICKS hrOPEN + 2           // количество кликов с момента включения
// ===
#ifdef SERIAL_DEBUG
#define hrDSCONVTIME hrOPEN + nValve*3      // DEBUG!!, таймаут на преобразование DS (можно менять удаленно)   
#define hrDSDEBUG hrDSCONVTIME + 1          // DEBUG!!, будем хранить время преобразования каждой DS'ки
#define hrVALVEDEBUG hrDSDEBUG + nSensor    // DEBUG!!, будем хранить длительность открытия каждого клапана
#endif // DEBUG_INFO
// ===

const int nSensor = sizeof(ds18b20) / sizeof(DSThermometer);  // считаем количество DS'ок
const int nValve = sizeof(valve) / sizeof(Valve);             // считаем количество клапанов

unsigned long msReinit;
uint16_t msGet, msLcd;
uint16_t msTimeout = DS_CONVTIME;
const char degC = 223;
char cbuf[] = { "     " };
bool mbMasterOK;

#ifdef SMART_RELAY
bool modbusON;
bool relayON = false;
#include <ReadDigKey.h>
ReadDigKey key;
#endif

#ifdef USE_OLED
#define LD_init() LD.init()
#define LD_clearDisplay() LD.clearDisplay()
#define LD_printNumber(a) LD.printNumber(a)
#define LD_printString_6x8(a,b,c) LD.printString_6x8(a, b, c)
#define LD_printString_12x16(a,b,c) LD.printString_12x16(a, b, c)
#define LD_printChar_6x8(a) LD.printString_6x8(a)
#define LD_printChar_12x16(a) LD.printString_12x16(a)
#else
#define LD_init()
#define LD_clearDisplay()
#define LD_printNumber(a)
#define LD_printString_6x8(a,b,c)
#define LD_printString_12x16(a,b,c)
#define LD_printChar_6x8(a)
#define LD_printChar_12x16(a)
#endif  // USE_OLED

#ifdef SERIAL_INFO
#ifndef SERIAL_CONFIG
#define SERIAL_CONFIG
#endif
#define PRINTLN(...) (Serial.println(__VA_ARGS__))
#define PRINT(...) (Serial.print(__VA_ARGS__))
#else
#define PRINTLN(xArg)
#define PRINT(zArg)
#endif  // SERIAL_INFO

#ifdef SERIAL_CONFIG
#define CFG_PRINTLN(...) (Serial.println(__VA_ARGS__))
#define CFG_PRINT(...) (Serial.print(__VA_ARGS__))
#else
#define CFG_PRINTLN(xArg)
#define CFG_PRINT(zArg)
#endif  // SERIAL_CONFIG

#ifdef SERIAL_DEBUG
#define DBG_PRINTLN(...) (Serial.println(__VA_ARGS__))
#define DBG_PRINT(...) (Serial.print(__VA_ARGS__))
#else
#define DBG_PRINTLN(xArg)
#define DBG_PRINT(zArg)
#endif  // SERIAL_DEBUG
