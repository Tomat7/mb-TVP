// *** поправить по необходимости! ***
#define DSPINS { 3, 4, 5, 6, 7 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 8, 9 }     // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)
#define MPX5010_PIN A1            // PIN на который подключен датчик MPX5010dp
#define LCD_ADDRESS 0x3F          // адрес LCD дисплея - необходимо найти с помощью i2c_scanner.ino
#define BMP280_ADDRESS 0x76       // адрес датчика BMP280 - необходимо найти с помощью i2c_scanner.ino
#define SERIALSPEED 115200        // скорость в последовательном порту

//#define ETHERNET_ENC28J60     // с регулятором мощности шилд enc28j60 использовать не рекомендуется
#define ETHERNET_WIZ5100        // если используется w5100 шилд, с регулятором мощности только он

#define ETHERNET_ID 0x08
// младший байт MAC адреса ** для каждого устройства в сети MAC должен быть уникальным **
// также смотрите и правьте MACADDRESS, MAC0-2, ETHERNET_IP в config.h

//#define ETHERNET_DHCP
// получить IP адрес динамически от DHCP сервера при старте
// скетч занимает больше памяти! особенно с модулем w5100
// если закоментировать то адрес будет назначен "принудительно"
// *** посмотреть ETHERNET_IP в config.h ***

#define PRESSURE_MPX    // скомпилировать с поддержкой датчика давления в кубе MPX5010dp
//#define PRESSURE_BMP    // скомпилировать с поддержкой датчика атмосферного давления BMP280
// посмотреть/перепроверить адрес BMP280_ADDRESS в файле config.h
// *** только один датчик давления может быть в скетче!! ***
// (можно и больше, но надо перелопатить нумерацию регистров МБ)

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
  for (int i = 0; i < dsCount; i++) initDS(i);

  LD.printString_6x8("VALVE on pin: ", LCDX1, 7);
  for (int i = 0; i < vCount; i++) initValve(i);  //

  msReinit = millis();
  //delay(1000);                          // Modbus Registers definitions
  mb.addHreg(hrSECONDS);                  // Like "alive" flag counter for future (for HeartBeat)
  mb.addHreg(hrPRESSURE);                 // Давление - атмосферное или избыточное (даже если без датчиков)
  mb.addHreg(hrDSCONVTIME, DS_CONVTIME);  // Время на преобразование для DS18B20 (DEBUG!!)
  printFreeRam();
  delay(5000);
  LD.clearDisplay();
}

void loop()
{
  for (int i = 0; i < dsCount; i++) ds18b20[i].check();
  for (int i = 0; i < vCount; i++) valve[i].control();
  mb.task();

  if (((millis() - msReinit) > msConvTimeout) && (!valve[0].Flow))
  {
    LD.printString_12x16("MB___", LCDX2, 6);
    msReinit = millis();

    msConvTimeout = mb.Hreg(hrDSCONVTIME);
    for (int i = 0; i < dsCount; i++) handleDS(i);
    for (int i = 0; i < vCount; i++) handleValve(i);

    mbHeartBeat();  // Проверяем "живость" Modbus мастера

#ifdef PRESSURE_BMP
    bmp280.check();
    mb.Hreg(hrPRESSURE, bmp280.Press_mmHg);
    String pressInfo = "TEMP " + String(bmp280.Temp_C, 2) + " DegC  PRESS : " + String(bmp280.Press_Pa, DEC) +
                     " Pa | " + String(bmp280.Press_mmHg, DEC) + " mmHg";
    Serial.println(pressInfo);
    dtostrf(mb.Hreg(hrPRESSURE), 5, 0, cbuf);
    LD.printString_12x16(cbuf, LCDX2, 3);
#endif

#ifdef PRESSURE_MPX
    int dPress_mmHg = analogRead(MPX5010_PIN);
    dPress_mmHg = (dPress_mmHg * 4 - 160) / 50;
    mb.Hreg(hrPRESSURE, dPress_mmHg);
    String pressInfo = "OverPressure: " + String(dPress_mmHg, DEC) + " mmHg";
    Serial.println(pressInfo);
    dtostrf(dPress_mmHg, 5, 0, cbuf);
    LD.printString_12x16(cbuf, LCDX2, 3);
#endif
  }
}

