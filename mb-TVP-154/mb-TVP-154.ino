// *** поправить по необходимости! ***
#define PLC_ID 0x05

#if PLC_ID == 0x02 
#define DSPINS { 3, 4, 6 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 7 }     // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
#define PRESSURE_BMP        // скомпилировать с поддержкой датчика атмосферного давления BMP280
#define BMP280_ADDRESS 0x76       // адрес датчика BMP280 - необходимо найти с помощью i2c_scanner.ino
#endif

#if PLC_ID == 0x05
#define DSPINS { 3, 4, 5 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 8, 9 }  // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
#define MPX5010_PIN A1      // PIN на который подключен датчик MPX5010dp
#define PRESSURE_MPX      // скомпилировать с поддержкой датчика давления в кубе MPX5010dp
#endif

// *** только один датчик давления может быть в скетче!! ***
// (можно и больше, но надо перелопатить нумерацию регистров МБ)

#define LCD_ADDRESS 0x3F    // адрес LCD дисплея - необходимо найти с помощью i2c_scanner.ino
#define SERIALSPEED 115200  // скорость в последовательном порту

#define ETHERNET_ENC28J60   // с регулятором мощности шилд enc28j60 использовать не рекомендуется
//#define ETHERNET_WIZ5100  // если используется w5100 шилд, с регулятором мощности только он

#define ETHERNET_ID PLC_ID
// младший байт MAC адреса ** для каждого устройства в сети MAC должен быть уникальным **
// также смотрите и правьте MACADDRESS, MAC0-2, ETHERNET_IP в config.h

//#define ETHERNET_DHCP
// получить IP адрес динамически от DHCP сервера при старте
// скетч занимает больше памяти! особенно с модулем w5100
// если закоментировать то адрес будет назначен "принудительно"
// *** посмотреть ETHERNET_IP в config.h ***

#include "config.h"
// *** заглянуть и поправить по необходимости! ***
// *** в нём описаны переменные, константы, #define, #include
// *** в новых версиях в functions.ino перенесены все процедуры

void setup()
{
  Serial.begin(SERIALSPEED);
  LD.init();
  LD.clearDisplay();
  const char* SketchInfo = SKETCHFILE;
  LD.printString_6x8(SketchInfo, 25, 0);   // 25 - подбирается экпериментально чтобы на экране смотрелось :-)

  delay(300);
  Serial.print(SketchInfo);
  Serial.println(F(SKETCHTIME));
  printFreeRam();
  setupNetMB();

#ifdef PRESSURE_BMP
  bmp280.init(BMP280_ADDRESS);
#endif

  LD.printString_6x8("DS on pin: ", LCDX1, 5);
  for (int i = 0; i < nSensor; i++) initDS(i);

  LD.printString_6x8("VALVE on pin: ", LCDX1, 7);
  for (int i = 0; i < nValve; i++) initValve(i);  //

  msReinit = millis();
  //delay(1000);                          // Modbus Registers definitions
  mb.addHreg(hrSECONDS);                  // Like "alive" flag counter for future (for HeartBeat)
  mb.addHreg(hrPRESSURE);                 // Давление - атмосферное или избыточное (даже если без датчиков)
  printFreeRam();
  delay(5000);
  LD.clearDisplay();
#ifdef DEBUG_INFO
  mb.addHreg(hrDSCONVTIME, DS_CONVTIME);  // Время на преобразование для DS18B20 (DEBUG!!)
#endif
}

void loop()
{
  for (int i = 0; i < nSensor; i++) ds18b20[i].check();
  for (int i = 0; i < nValve; i++) valve[i].control();
  mb.task();

  if (millis() - msReinit > msTimeout)
  {
    if ((!valve[0].Flow && !valve[1].Flow) || (millis() - msReinit > 1000))
    {
      LD.printString_12x16("MB___", LCDX2, 6);
      for (int i = 0; i < nSensor; i++) updateDS(i);
      for (int i = 0; i < nValve; i++) updateValve(i);
      updatePressure();
      msReinit = millis();
      checkMBmaster();      // Проверяем "живость" Modbus мастера
#ifdef DEBUG_INFO
      msTimeout = mb.Hreg(hrDSCONVTIME);
#endif
    }
  }
}

