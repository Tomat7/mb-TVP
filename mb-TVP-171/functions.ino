
void setupNetMB()
{
  //Config Modbus IP
  LD.printString_6x8("MAC: ", LCDX1, 1);
  for (byte i = 0; i < 6; ++i)
  {
    char mh[2];
    String macStr = String(mac[i], HEX);
    macStr.toUpperCase();
    macStr.toCharArray(mh, 3);
    LD.printString_6x8(mh);
    if (i < 5) LD.printString_6x8(":");
  }

#ifdef SERIAL_CONFIG
  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i)
  {
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(':');
  }
  Serial.println();
#endif

#ifndef ETHERNET_DHCP
  const byte ip[] = { ETHERNET_IP };
  mb.config(mac, ip);
#else
  mb.config(mac);
#endif

#ifdef ETHERNET_ENC28J60
#ifdef SERIAL_CONFIG
  ether.printIp("NC28J60 IP: ", ether.myip);
  ether.printIp("MASK: ", ether.netmask);
  ether.printIp("GW: ", ether.gwip);
  //ether.printIp("DNS: ", ether.dnsip);
#endif
  LD.printString_6x8("enc_IP: ", LCDX1, 3);
  for (byte i = 0; i < 4; ++i)
  {
    LD.printNumber((long)ether.myip[i]);
    if (i < 3) LD.printString_6x8(".");
  }
#endif

#ifdef ETHERNET_WIZ5100
#ifdef SERIAL_CONFIG
  Serial.print(F("WIZ5100 IP: "));
  Serial.println(Ethernet.localIP());
#endif
  LD.printString_6x8("wiz_ip: ", LCDX1, 3);
  for (byte i = 0; i < 4; ++i)
  {
    LD.printNumber((long)Ethernet.localIP()[i]);
    if (i < 3) LD.printString_6x8(".");
  }
#endif
}

void checkMBmaster()
{
  // если мастер онлайн - он должен записывать 0 в регистр SECOnSensor
  // это будет признаком "живости" Мастера Modbus'а для модуля
  // и наоборот: не 0 в SECONDS - признак "живости" модуля для Мастера
  // хотя Мастеру логичнее отслеживать "живость" по GetQuality
  uint16_t secUptime = (uint16_t)(msReinit / 1000);
  if (mb.Hreg(hrSECONDS) == 0) mb.Hreg(hrSECONDS, secUptime);
  if ((secUptime - mb.Hreg(hrSECONDS)) < MB_TIMEOUT)
  {
    LD.printString_12x16("MB_OK", LCDX2, 6);
    mbMasterOK = true;
  }
  else                // если мастера нет больше MB_TIMEOUT секунд - пишем на экране и в порт
  {
    LD.printString_12x16("  OFF", LCDX2, 6);
#ifdef SERIAL_INFO
    Serial.print("Master OFFline ");
    Serial.println(mb.Hreg(hrSECONDS));
#endif
    mbMasterOK = false;
  }
}

void initDS(int i)
{
  ds18b20[i].init(DS_CONVTIME);
  mb.addHreg(hrTEMP + i);                // Модбас регистр - значение температуры
  int pins[] = DSPINS;
  LD.printNumber((long)pins[i]);
  if (i < (nSensor - 1)) LD.printString_6x8(", ");
#ifdef SERIAL_CONFIG
  String sInfo = "ID " + String(i, DEC) + "|Connected " + String(ds18b20[i].Connected, DEC);
  Serial.println(sInfo);
#endif
#ifdef DEBUG_INFO
  mb.addHreg(hrDSDEBUG + i);  // Модбас регистр - длительность преобразования !DEBUG!
#endif
}

void initValve(int i)
{
  byte hrOpen = hrOPEN + i * 3;         // сдвиг если клапанов больше одного
  byte hrClose = hrCLOSE + i * 3;
  byte hrClicks = hrCLICKS + i * 3;
  valve[i].init();
  mb.addHreg(hrOpen);         // Длительность нахождения клапана в ОТКРЫТОМ состоянии
  mb.addHreg(hrClose);        // Длительность нахождения клапана в ЗАКРЫТОМ состоянии
  mb.addHreg(hrClicks);       // количество кликов с начала работы (для расчета объема)
  int pins[] = VALVEPINS;
  LD.printNumber((long)pins[i]);
  if (i < (nValve - 1)) LD.printString_6x8(", ");
#ifdef DEBUG_INFO
  mb.addHreg(hrVALVEDEBUG + i);  // DEBUG !! длительность крайнего открытия
#endif
}

void initPressure()
{
#ifdef PRESSURE_BMP
  bmp280.init();
#endif
#ifdef PRESSURE_MPX
  mpx5010dp.init(1, Pins, Vals);
#endif
}

void updateDS(int i)
{
  float t = ds18b20[i].Temp;
  mb.Hreg(hrTEMP + i, round(t * 100)); // заносим в регистр Modbus (температура * 100)
  dtostrf(t, 5, 1, cbuf);
  LD.printString_12x16(cbuf, 0, (i * 3));
#ifdef SERIAL_INFO
  String dsInfo = "DS " + String(i, DEC) + ": " + String(t, 2) + " | parasite: " +
                  String(ds18b20[i].Parasite, DEC) + " | " + String(ds18b20[i].TimeConv, DEC);
  Serial.println(dsInfo);
#endif
#ifdef DEBUG_INFO
  mb.Hreg(hrDSDEBUG + i, ds18b20[i].TimeConv);  // DEBUG!
#endif
}

void updateValve(int i)
{
  byte hrOpen = hrOPEN + i * 3;         // сдвиг если клапанов больше одного
  byte hrClose = hrCLOSE + i * 3;
  byte hrClicks = hrCLICKS + i * 3;
  if (mbMasterOK)
  {
    //mb.Hreg(hrVALVEDEBUG + i, valve[i].lastON); // сохраняем залёты по времени (если чаще чем клики клапана??)
    if (mb.Hreg(hrClicks) == 65535) valve[i].Clicks = 0;
    else if (mb.Hreg(hrClicks) > valve[i].Clicks) valve[i].Clicks = mb.Hreg(hrClicks);
    mb.Hreg(hrClicks, valve[i].Clicks); // сохраняем сколько всего накликали
  }
  else
  {
    //mb.Hreg(hrLAST_DROP, 0);
    mb.Hreg(hrOpen, 0);            // а если Мастера нет - прекращаем клацать клапаном
    mb.Hreg(hrClose, 65535);
  }
  valve[i].setTime(mb.Hreg(hrClose), mb.Hreg(hrOpen)); // задаем длительность в открытом и закрытом состоянии
  dtostrf(mb.Hreg(hrOpen), 4, 0, cbuf);
  LD.printString_6x8(cbuf, LCDX2, i); // показываем длительность в отрытом на дисплее
  dtostrf(mb.Hreg(hrClose), 5, 0, cbuf);
  LD.printString_6x8(cbuf, LCDX2 + 32, i); // показываем длительность в закрытом на дисплее
#ifdef SERIAL_INFO
  String vInfo = "VALVE " + String(i, DEC) + ": On " + String(mb.Hreg(hrOpen), DEC) +
                 ": Off " + String(mb.Hreg(hrClose), DEC) + " ms | clicks: " + String(valve[i].Clicks, DEC);
  Serial.println(vInfo);
#endif
#ifdef DEBUG_INFO
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
  LD.printString_12x16(cbuf, LCDX2, 3);
#ifdef SERIAL_INFO
  String pInfo = "TEMP " + String(bmp280.Temp_C, 2) + " DegC  PRESS : " + String(bmp280.Press_Pa, DEC) +
                 " Pa | " + String(bmp280.Press_mmHg, DEC) + " mmHg";
  Serial.println(pInfo);
#endif
#endif

#ifdef PRESSURE_MPX
  mpx5010dp.check();
  int Press_mmHg = (mpxRAW * 4 - 160) / 50;
  mb.Hreg(hrPRESSURE, Press_mmHg);
  dtostrf(Press_mmHg, 5, 0, cbuf);
  LD.printString_12x16(cbuf, LCDX2, 3);
#ifdef SERIAL_INFO
  String pInfo = "OverPressure: " + String(Press_mmHg, DEC) + " mmHg";
  Serial.println(pInfo);
#endif
#endif
}

void printFreeRam()
{
#ifdef SERIAL_CONFIG
  Serial.print(F("Free RAM: "));
  extern int __heap_start, *__brkval;
  int v, r;
  r = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  Serial.println(r);
#endif
}

void(*resetFunc) (void) = 0;    // Перезагрузка Ардуины

