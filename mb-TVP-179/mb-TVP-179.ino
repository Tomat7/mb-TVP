
#define PLC_ID 0x08 // "адрес" устройства, от него формируются MAC и IP адреса и выбираются шаблоны модулей
// 0x02 - BMP280, 0x05 - MPX5010, 0x07 - ESP32, 0x08 - ESP8266

#include "config_plc.h"     // конфигурация контроллеров
#include "config_net.h"     // конфигурация сети
#include "config.h"         // описание переменных, констант
// *** обязательно заглянуть и поправить по необходимости! ***

void setup()
{
    Serial.begin(SERIALSPEED);
    LD_init();
    LD_clearDisplay();

    String strVersion = SKETCHFILE;
    String strVer = strVersion.substring(1 + strVersion.lastIndexOf('\\'));
    char chVer[16];
    strVer.toCharArray(chVer, 16);
    LD_printString_6x8(chVer, LCDX1, 0);

    delay(300);
    CFG_PRINT(strVersion);
    CFG_PRINTLN(F(SKETCHTIME));

    printFreeRam();
    setupNetMB();

    mb.addHreg(hrSECONDS);                  // Like "alive" flag counter for future (for HeartBeat)
    initDS();
    initValve();
    initPressure();

    msReinit = millis();
    //delay(1000);                          // Modbus Registers definitions
    //printFreeRam();
    delayMs(5000);
    LD_clearDisplay();

#ifdef SERIAL_DEBUG
    mb.addHreg(hrDSCONVTIME, DS_CONVTIME);  // Время на преобразование для DS18B20 (DEBUG!!)
#endif
#ifdef SMART_RELAY
    key.add_key(BUTTON_MODE);
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
            LD_printString_12x16("MB___", LCDX2, 6);
            for (int i = 0; i < nSensor; i++) updateDS(i);
            for (int i = 0; i < nValve; i++) updateSwitch(i);
            updatePressure();
            msReinit = millis();
            checkMBmaster();      // Проверяем "живость" Modbus мастера
#ifdef SERIAL_DEBUG
            msTimeout = mb.Hreg(hrDSCONVTIME);
#endif
        }
    }
#ifdef SMART_RELAY
    key.readkey();						// Чтение клавиатуры
#endif // SMART_RELAY
}
