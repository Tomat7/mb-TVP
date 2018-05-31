// *** поправить по необходимости! ***
#define ETHERNET_ENC28J60
//#define ETHERNET_WIZ5100
#define ETHERNET_MACID 0x02
//#define ETHERNET_DHCP

#define OWPINS { 3, 4, 6 }  // на эти пины подключается по **одному** датчику DS18B20
#define VALVEPINS { 7 }     // на эти пины подключаются обвязка клапанов (оптопара/транзистор, SSR и тд.)

#include "config.h"
// *** заглянуть и поправить по необходимости! ***
// *** в нём описаны переменные, константы, #define, #include
// *** в новых версиях в него перенесены все функции
// *** в functions.ino перенесены все процедуры

void setup()
{
  Serial.begin(SERIALSPEED);
  LD.init();
  LD.clearDisplay();
  const char* SketchInfo = SKETCHFILE;
  LD.printString_6x8(SketchInfo, 25, 0);
  bmp280.begin();

  delay(300);
  Serial.print(SketchInfo);
  Serial.println(F(SKETCHTIME));
  printFreeRam();
  setupNetMB();

  LD.printString_6x8("DS on pin: ", LCDX1, 5);
  for (int i = 0; i < sensCount; i++) initDS(i);

  valve[0].init(0, 100);
  int pins[] = VALVEPINS;
  LD.printString_6x8("VALVE on pin: ", LCDX1, 7);
  LD.printNumber((long)pins[0]);

  msReinit = millis();
  //delay(1000);                           // Modbus Registers definitions
  mb.addHreg(hrConvTimeout, DS_CONVTIME); // default timeout for DS18B20 conversation
  mb.addHreg(hrCLICK_INTERVAL);           // Valve OFF-time - регулирует скорость отбора
  mb.addHreg(hrLAST_DROP);     // DEBUG?? // длительность крайнего открытия клапана (100 мс + залёты)
  mb.addHreg(hrCLICKS);                   // количество кликов с начала работы (для расчета объема)
  mb.addHreg(hrSECONDS);                  // Like "alive" flag counter for future (for HeartBeat)
  //valve[0].setTime(1000);
  printFreeRam();
  delay(5000);
  LD.clearDisplay();
}

void loop()
{
  valve[0].control();
  mb.task();
  for (int i = 0; i < sensCount; i++) sensor[i].check();

  if (((millis() - msReinit) > msConvTimeout) && (!valve[0].Flow))
  {
    LD.printString_12x16("MB___", LCDX2, 6);
    msReinit = millis();

    for (int i = 0; i < sensCount; i++) handleDS(i);

    mbHeartBeat();  // Проверяем "живость" Modbus мастера

    msConvTimeout = mb.Hreg(hrConvTimeout);
    valve[0].setTime(mb.Hreg(hrCLICK_INTERVAL));
    dtostrf(mb.Hreg(hrCLICK_INTERVAL), 5, 0, cbuf);
    LD.printString_12x16(cbuf, LCDX2, 0);
    dtostrf(mb.Hreg(hrLAST_DROP), 5, 0, cbuf);
    LD.printString_12x16(cbuf, LCDX2, 3);

    double T, P;
    char result = bmp280.startMeasurment();
    delay(result);
    result = bmp280.getTemperatureAndPressure(T, P);

    /*
        Serial.print("Pressure = ");
        Serial.print(bmp280.readPressure());
        Serial.println(" Pa");
    */
  }
}

