#define SKETCHFILE __FILE__ " "
#define SKETCHTIME __DATE__ " " __TIME__
#define SKETCHVERSION SKETCHFILE SKETCHTIME
#define MACADDRESS MAC0, MAC1, 0xBE, 0xEF, 0x30, ETHERNET_MACID
#define IPBASE 192, 168, 1, 30

//#include <iarduino_Pressure_BMP.h>
//iarduino_Pressure_BMP bmp280; 

//#include <Adafruit_Sensor.h>
//#include <Adafruit_BMP280.h>
//Adafruit_BMP280 bmp280; // I2C

#include "BMP280.h"
BMP280 bmp280;

#include "DStemp.h"
DSThermometer sensor[] = OWPINS;
#include "HDvalve.h"
Valve valve[] = VALVEPINS;
// ===
#define MAXPOWER 3000
#define SERIALSPEED 115200
#define DS_CONVTIME 750   // msec
#define MB_TIMEOUT 50     // sec
// ===
#include <ASOLED.h>
#define LCDX1 1
#define LCDX2 67
//#include "RegPower.h"
//#include "power.h"
// ===
#define hrConvTimeout sensCount
#define hrCLICK_INTERVAL sensCount + 1
#define hrLAST_DROP sensCount + 2
#define hrCLICKS sensCount + 3
#define hrSECONDS sensCount + 4
// ===
#define hrPset sensCount + 1
#define hrPnow sensCount + 2
// ===
#ifndef ETHERNET_MACID
#include <EEPROM.h>
#endif
// ===
#ifdef ETHERNET_DHCP
#define MAC0 0x10
#else
#define MAC0 0x00
#endif
// ===
#ifdef ETHERNET_ENC28J60
#include <EtherCard.h>
#include <ModbusIP_ENC28J60.h>
#define MAC1 0x28
#endif
// ===
#ifdef ETHERNET_WIZ5100
#include <Ethernet.h>
#include <ModbusIP.h>
#define MAC1 0x51
#endif
// ===
#include <Modbus.h>
ModbusIP mb;
//==================================
const int sensCount = sizeof(sensor) / sizeof(DSThermometer);
unsigned long msReinit;
uint16_t msGet, msLcd;
uint16_t msConvTimeout = DS_CONVTIME;
const char degC = 223;
char cbuf[] = {"     "};
const uint8_t mac[] = { MACADDRESS };
const uint8_t ip[] = { IPBASE + ETHERNET_MACID };
// ==================================

