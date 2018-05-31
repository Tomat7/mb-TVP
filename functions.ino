
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

  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i)
  {
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(':');
  }
  Serial.println();

#ifndef ETHERNET_DHCP
  const byte ip[] = { ETHERNET_IP };
  mb.config(mac, ip);
#else
  mb.config(mac);
#endif

#ifdef ETHERNET_ENC28J60
  ether.printIp("NC28J60 IP: ", ether.myip);
  ether.printIp("MASK: ", ether.netmask);
  ether.printIp("GW: ", ether.gwip);
  //ether.printIp("DNS: ", ether.dnsip);
  LD.printString_6x8("enc_IP: ", LCDX1, 3);
  for (byte i = 0; i < 4; ++i)
  {
    LD.printNumber((long)ether.myip[i]);
    if (i < 3) LD.printString_6x8(".");
  }
#endif

#ifdef ETHERNET_WIZ5100
  Serial.print(F("WIZ5100 IP: "));
  Serial.println(Ethernet.localIP());
  LD.printString_6x8("wiz_ip: ", LCDX1, 3);
  for (byte i = 0; i < 4; ++i)
  {
    LD.printNumber((long)Ethernet.localIP()[i]);
    if (i < 3) LD.printString_6x8(".");
  }
#endif
}

void mbHeartBeat()
{
  // если мастер онлайн - он должен записывать 0 в регистр SECONDS
  // это будет признаком "живости" Мастера Modbus'а для модуля
  // и наоборот: не 0 в SECONDS - признак "живости" модуля для Мастера
  // хотя Мастеру логичнее отслеживать "живость" по GetQuality

  if (mb.Hreg(hrSECONDS) == 0) mb.Hreg(hrSECONDS, msReinit / 1000);
  if (((uint16_t)(msReinit / 1000) - mb.Hreg(hrSECONDS)) < MB_TIMEOUT)
  {
    //mb.Hreg(hrLAST_DROP, valve[0].lastON);   // сохраняем залёты по времени (если чаще чем клики клапана??)
    //mb.Hreg(hrCLICKS, valve[0].Clicks);      // сохраняем сколько всего накликали
    LD.printString_12x16("MB_OK", LCDX2, 6);
    mbMasterOK = true;
  }
  else                // если мастера нет больше MB_TIMEOUT секунд - поднимаем флаг
  {
    //mb.Hreg(hrLAST_DROP, 0);
    //mb.Hreg(hrCLICK_INTERVAL, 0);
    LD.printString_12x16("  OFF", LCDX2, 6);
    Serial.print("Master OFFline ");
    Serial.println(mb.Hreg(hrSECONDS));
    mbMasterOK = false;
  }
}

void initDS(int i)
{
  ds18b20[i].init(DS_CONVTIME);
  mb.addHreg(hrTEMP + i);                // Модбас регистр - значение температуры
  mb.addHreg(hrDSDEBUG + i);  // Модбас регистр - длительность преобразования !DEBUG!
  int pins[] = DSPINS;
  String dsInfo = "ID " + String(i, DEC) + "|Connected " + String(ds18b20[i].Connected, DEC);
  Serial.println(dsInfo);
  LD.printNumber((long)pins[i]);
  if (i < (dsCount - 1)) LD.printString_6x8(", ");
}

void initValve(int i)
{
  int hrBase = hrVALVE + i * 3;
  valve[i].init(0, V_OPENTIME);
  mb.addHreg(hrBase);                // Valve OFF-time - регулирует скорость отбора
  mb.addHreg(hrBase + 1); // DEBUG?? // длительность крайнего открытия клапана (100 мс + залёты)
  mb.addHreg(hrBase + 2);            // количество кликов с начала работы (для расчета объема)
  int pins[] = VALVEPINS;
  LD.printNumber((long)pins[i]);
  if (i < (vCount - 1)) LD.printString_6x8(", ");
}

void handleDS(int i)
{
  float t = ds18b20[i].Temp;
  mb.Hreg(hrTEMP + i, round( t * 100)); // заносим в регистр Modbus (температура * 100)
  mb.Hreg(hrDSDEBUG + i, ds18b20[i].TimeConv);  // DEBUG!
  String dsInfo = "DS " + String(i, DEC) + ": " + String(t, 2) + " | parasite: " +
                  String(ds18b20[i].Parasite, DEC) + " | " + String(ds18b20[i].TimeConv, DEC);
  Serial.println(dsInfo);
  dtostrf(t, 4, 1, cbuf);
  LD.printString_12x16(cbuf, 0, (i * 3));
}

void handleValve(int i)
{
  int hrBase = hrVALVE + i * 3;         // адрес регистра в котором хранится длительность стопа клапана
  if (mbMasterOK)
  {
    mb.Hreg(hrBase + 1, valve[i].lastON); // сохраняем залёты по времени (если чаще чем клики клапана??)
    mb.Hreg(hrBase + 2, valve[i].Clicks); // сохраняем сколько всего накликали
  } else
  {
    //mb.Hreg(hrLAST_DROP, 0);
    mb.Hreg(hrBase, 0);              // если Мастера нет - прекращаем клацать клапаном
  }
  valve[i].setTime(mb.Hreg(hrBase)); // задаем время стопа клапану
  dtostrf(mb.Hreg(hrBase), 5, 0, cbuf);
  LD.printString_6x8(cbuf, LCDX2+32, i);   // показываем это время на дисплее
  String vInfo = "VALVE " + String(i, DEC) + ": off " + String(mb.Hreg(hrBase), DEC) + " ms | lastOn " +
                 String(valve[i].lastON, DEC) + " ms | clicks: " + String(valve[i].Clicks, DEC);
  Serial.println(vInfo);
}

void printFreeRam ()
{
  Serial.print(F("Free RAM: "));
  extern int __heap_start, *__brkval;
  int v, r;
  r = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  Serial.println(r);
}

