// ниже "шаблоны" модулей - поправить под свои нужды
// только один датчик давления может быть в скетче! иначе, надо перелопатить нумерацию регистров Модбаса
#if PLC_ID == 0x02          // AVR, BMP280
#define USE_OLED
#define OLED_SH1106
#define DSPINS { 3, 4, 6 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 7, 8 }  // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
#define PRESSURE_BMP        // скомпилировать с поддержкой датчика атмосферного давления BMP280
#define BMP280_ADDRESS 0x76 // адрес датчика BMP280 - необходимо найти с помощью i2c_scanner.ino
#define ETHERNET_ENC28J60   // с регулятором мощности шилд enc28j60 использовать не рекомендуется
#ifndef __AVR__
#error "Wrong module defined. Use Arduino Nano or change PLC_ID."
#endif
#endif  // PLC_ID == 0x02
// ===
#if PLC_ID == 0x05          // AVR, MPX5010
#define USE_OLED
#define DSPINS { 3, 4, 5 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 8, 9 }  // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
#define PRESSURE_MPX        // скомпилировать с поддержкой датчика давления в кубе MPX5010dp
#define MPX5010_PIN A1      // PIN на который подключен датчик MPX5010dp
#define ETHERNET_ENC28J60   // с регулятором мощности шилд enc28j60 использовать не рекомендуется
#ifndef __AVR__
#error "Wrong module defined. Use Arduino Nano or change PLC_ID."
#endif
#endif  // PLC_ID == 0x05
// ===
#if PLC_ID == 0x07          // ESP32 
#define USE_OLED
#define OLED_SH1106
#define DSPINS { 32, 33, 25 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 12, 13, 18 }  // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
//#define PRESSURE_BMP        // скомпилировать с поддержкой датчика атмосферного давления BMP280
//#define BMP280_ADDRESS 0x76 // адрес датчика BMP280 - необходимо найти с помощью i2c_scanner.ino
//#define PRESSURE_MPX        // скомпилировать с поддержкой датчика давления в кубе MPX5010dp
//#define MPX5010_PIN 34      // PIN на который подключен датчик MPX5010dp
#ifndef ESP32
#error Wrong module defined. Use ESP32 or change PLC_ID.
#endif
#endif  // PLC_ID == 0x07
// ===
#if PLC_ID == 0x08          // ESP8266, SmartSocket
#define SMART_RELAY
#define OLED_SH1106
#define DSPINS { 14 }         // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 13, 12 }  // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
//#define PRESSURE_BMP        // скомпилировать с поддержкой датчика атмосферного давления BMP280
//#define BMP280_ADDRESS 0x76 // адрес датчика BMP280 - необходимо найти с помощью i2c_scanner.ino
//#define PRESSURE_MPX        // скомпилировать с поддержкой датчика давления в кубе MPX5010dp
//#define MPX5010_PIN 34      // PIN на который подключен датчик MPX5010dp
#define LED_INFO 0
#define LED_RELAY 1
#define BUTTON_MODE 0
#ifndef ESP8266
#error "Wrong module defined. Use ESP8266 or change PLC_ID."
#endif
#endif  // PLC_ID == 0x08
// ===

