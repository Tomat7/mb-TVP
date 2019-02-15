void initDS()
{
#ifdef __AVR__
    LD_printString_6x8("DS on pin: ", LCDX1, 5);
#else
    LD_printString_6x8("DS: ", LCDX1, 5);
#endif

    for (int i = 0; i < nSensor; i++)
    {
        ds18b20[i].init(DS_CONVTIME);
        mb.addHreg(hrTEMP + i);                // Модбас регистр - значение температуры
        int pins[] = DSPINS;
        LD_printNumber((long)pins[i]);
        if (i < (nSensor - 1)) LD_printChar_6x8(", ");
#ifdef SERIAL_CONFIG
        ds18b20[i].printConfig();
        String sInfo = "ID " + String(i, DEC) + "|Connected " + String(ds18b20[i].Connected, DEC);
        Serial.println(sInfo);
#endif
#ifdef SERIAL_DEBUG
        mb.addHreg(hrDSDEBUG + i);  // Модбас регистр - длительность преобразования !DEBUG!
#endif
    }
}

void initValve()
{
#ifdef __AVR__
    LD_printString_6x8("VALVE on pin: ", LCDX1, 7);
#else
    LD_printString_6x8("VALVE: ", LCDX1, 7);
#endif

    for (int i = 0; i < nValve; i++)
    {
        byte hrOpen = hrOPEN + i * 3;         // сдвиг если клапанов больше одного
        byte hrClose = hrCLOSE + i * 3;
        byte hrClicks = hrCLICKS + i * 3;
        valve[i].init();
        mb.addHreg(hrOpen);         // Длительность нахождения клапана в ОТКРЫТОМ состоянии
        mb.addHreg(hrClose);        // Длительность нахождения клапана в ЗАКРЫТОМ состоянии
        mb.addHreg(hrClicks);       // количество кликов с начала работы (для расчета объема)
        int pins[] = VALVEPINS;
        LD_printNumber((long)pins[i]);
        if (i < (nValve - 1)) LD_printChar_6x8(", ");
#ifdef SERIAL_DEBUG
        mb.addHreg(hrVALVEDEBUG + i);  // DEBUG !! длительность крайнего открытия
#endif
    }
}

void initPressure()
{
    mb.addHreg(hrPRESSURE);     // Давление - атмосферное или избыточное (даже если без датчиков)
#ifdef PRESSURE_BMP
    bmp280.init();
#endif
#ifdef PRESSURE_MPX
    mpx5010dp.init();
#endif
}

void updateDS(int i)
{
    float t = ds18b20[i].Temp;
    mb.Hreg(hrTEMP + i, round(t * 100)); // заносим в регистр Modbus (температура * 100)
    dtostrf(t, 5, 1, cbuf);
    LD_printString_12x16(cbuf, 0, (i * 3));
    if (ds18b20[i].Parasite) LD_printChar_6x8("`");
#ifdef SERIAL_INFO
    String dsInfo = "DS " + String(i, DEC) + ": " + String(t, 2) + " | parasite: " +
        String(ds18b20[i].Parasite, DEC) + " | " + String(ds18b20[i].TimeConv, DEC);
    Serial.println(dsInfo);
#endif
#ifdef SERIAL_DEBUG
    mb.Hreg(hrDSDEBUG + i, ds18b20[i].TimeConv);  // DEBUG!
#endif
}

void updateSwitch(int i)
{
#ifdef SMART_RELAY
    checkButton();
    updateRelay(i); // нужна только для SMART_RELAY
#else
    updateValve(i);
#endif
}

void updateRelay(int i)
{
#ifdef LED_INFO
    uint16_t msOff, msOn;
    if (i == LED_INFO)      // led_info реверсивный!
    {
        if (!modbusON) { msOff = 0; msOn = 0; }					// manual mode
        else if (!wifiConnectOK) { msOff = 100; msOn = 100; }	// no Wi-Fi at all
        else if (!mbMasterOK) { msOff = 100; msOn = 1500; }		// no Modbus master
        else { msOff = 1000; msOn = 1000; }						// MB - ok, Master - ok.
    }
    else
    {
        if (mbMasterOK && modbusON)
        {
            msOff = mb.Hreg(hrCLOSE);
            msOn = mb.Hreg(hrOPEN);
        }
        else if (relayON) { msOff = 0; msOn = 65500; }
        else { msOff = 65500; msOn = 0; }
    }
    PRINT(i); PRINT(" "); PRINT(msOff); PRINT(" "); PRINTLN(msOn);
    valve[i].setTime(msOff, msOn);
#endif  // LED_INFO
}

void updateValve(int i)
{
    byte hrOpen = hrOPEN + i * 3;       // сдвиг если клапанов больше одного
    byte hrClose = hrCLOSE + i * 3;
    byte hrClicks = hrCLICKS + i * 3;
    if (mbMasterOK)
    {
        if (mb.Hreg(hrClicks) == 65535) valve[i].Clicks = 0;
        else if (mb.Hreg(hrClicks) > valve[i].Clicks) valve[i].Clicks = mb.Hreg(hrClicks);
        mb.Hreg(hrClicks, valve[i].Clicks); // сохраняем сколько всего накликали
    }
    else
    {
        mb.Hreg(hrOpen, 0);            // а если Мастера нет - прекращаем клацать клапаном
        mb.Hreg(hrClose, 65535);
    }
    valve[i].setTime(mb.Hreg(hrClose), mb.Hreg(hrOpen)); // задаем длительность в открытом и закрытом состоянии
    dtostrf(mb.Hreg(hrOpen), 4, 0, cbuf);
    LD_printString_6x8(cbuf, LCDX2, i); // показываем на дисплее длительность в отрытом состоянии 
    dtostrf(mb.Hreg(hrClose), 5, 0, cbuf);
    LD_printString_6x8(cbuf, LCDX2 + 32, i); // показываем на дисплее длительность в закрытом состоянии 
#ifdef SERIAL_INFO
    String vInfo = "VALVE " + String(i, DEC) + ": On " + String(mb.Hreg(hrOpen), DEC) +
        ": Off " + String(mb.Hreg(hrClose), DEC) + " ms | clicks: " + String(valve[i].Clicks, DEC);
    Serial.println(vInfo);
#endif
#ifdef SERIAL_DEBUG
    vInfo = vInfo + "lastOn " + String(valve[i].lastON, DEC) + " ms | Off " + String(valve[i].lastOFF, DEC) + " ms";
    Serial.println(vInfo);
    mb.Hreg(hrVALVEDEBUG + i, valve[i].lastON);
#endif
}

void updatePressure()
{
#ifdef PRESSURE_BMP
    bmp280.check();
    mb.Hreg(hrPRESSURE, bmp280.Press_mmHg);
    dtostrf(mb.Hreg(hrPRESSURE), 5, 0, cbuf);
    LD_printString_12x16(cbuf, LCDX2, 3);
#ifdef SERIAL_INFO
    String pInfo = "TEMP " + String(bmp280.Temp_C, 2) + " DegC  PRESS : " + String(bmp280.Press_Pa, DEC) +
        " Pa | " + String(bmp280.Press_mmHg, DEC) + " mmHg";
    Serial.println(pInfo);
#endif
#endif

#ifdef PRESSURE_MPX
    mpx5010dp.check();
    int Press_mmHg = mpx5010dp.read();
    mb.Hreg(hrPRESSURE, Press_mmHg);
    Press_mmHg = (Press_mmHg * 4 - 160) / 50;
    dtostrf(Press_mmHg, 5, 0, cbuf);
    LD_printString_12x16(cbuf, LCDX2, 3);
#ifdef SERIAL_INFO
    String pInfo = "OverPressure: " + String(Press_mmHg, DEC) + " mmHg";
    Serial.println(pInfo);
#endif
#endif
}

