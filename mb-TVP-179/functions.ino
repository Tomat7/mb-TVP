
void setupNetMB()   //Config Modbus IP
{
    serialMAC();
    displayMAC();
    initETH();
    serialIP();
    displayIP();
}

void checkMBmaster()
{
	checkWiFi();
	// если мастер онлайн - он должен записывать 0 в регистр SECOnSensor
    // это будет признаком "живости" Мастера Modbus'а для модуля
    // и наоборот: не 0 в SECONDS - признак "живости" модуля для Мастера
    // хотя Мастеру логичнее отслеживать "живость" по GetQuality
    uint16_t secUptime = (uint16_t)(msReinit / 1000);
    if (mb.Hreg(hrSECONDS) == 0) mb.Hreg(hrSECONDS, secUptime);
    if ((secUptime - mb.Hreg(hrSECONDS)) < MB_TIMEOUT)
    {
        LD_printString_12x16("MB_OK", LCDX2, 6);
        mbMasterOK = true;
    }
    else                // если мастера нет больше MB_TIMEOUT секунд - пишем на экране и в порт
    {
        LD_printString_12x16("  OFF", LCDX2, 6);
#ifdef SERIAL_INFO
        PRINT("Master OFFline ");
        PRINTLN(mb.Hreg(hrSECONDS));
#endif
        mbMasterOK = false;
    }
}

void checkButton()
{
#ifdef SMART_RELAY
    if (!modbusON && key.shot_press())
    {
        relayON = !relayON;
        PRINTLN("relayON");
    }
    if (key.long_press())
    {
        modbusON = !modbusON;
        PRINTLN("modbusOFF");
    }
    if (modbusON)
    {
        relayON = false;
        PRINTLN("modbusON");
    }
#endif  // SMART_RELAY
}

void delayMs(uint32_t msWait)
{
#ifdef LED_INFO
	uint32_t msBegin = millis();
	while ((millis() - msBegin) < msWait)
	{
		valve[LED_INFO].control();
        delay(20);
	}
#else
	delay(msWait);
#endif
}


void printFreeRam()
{
#if defined(SERIAL_CONFIG) && defined(__AVR__)
    Serial.print(F("Free RAM: "));
    extern int __heap_start, *__brkval;
    int v, r;
    r = (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    Serial.println(r);
#endif
}

/*
  int *intFloat(float f, byte d)
  {
  static int intF[2];
  intF[0] = (int)f;                          // compute the integer part of the float
  intF[1] = (int)((f - (float)intF[0]) * d * 10); // compute 1 decimal places (and convert it to int)
  return intF;
  }
*/
void(*resetFunc) (void) = 0;    // Перезагрузка Ардуины

