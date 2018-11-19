
void setupNetMB()
{
  //Config Modbus IP
  /*
    #ifndef ETHERNET_MACID
    macID = EEPROM.read(EEPROM.length() - 1);
    #endif
  */
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
  /*
      LD.printString_6x8("cfg_IP: ", lcdX, 2);
      for (byte i = 0; i < 4; ++i)
      {
        LD.printNumber((long)ip[i]);
        if (i < 3) LD.printString_6x8(".");
      }
  */
#ifdef ETHERNET_DHCP
  mb.config(mac);
#else
  mb.config(mac, ip);
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
  //
  if (mb.Hreg(hrSECONDS) == 0) mb.Hreg(hrSECONDS, msReinit / 1000);
  if (((uint16_t)(msReinit / 1000) - mb.Hreg(hrSECONDS)) < MB_TIMEOUT)
  {
    mb.Hreg(hrLAST_DROP, valve[0].lastON);   // сохраняем залёты по времени (если чаще чем клики клапана??)
    mb.Hreg(hrCLICKS, valve[0].Clicks);      // сохраняем сколько всего накликали
    LD.printString_12x16("MB_OK", LCDX2, 6);
  }
  else                // если мастера нет больше XX секунд - глушим щелкунчика
  {
    mb.Hreg(hrLAST_DROP, 0);
    mb.Hreg(hrCLICK_INTERVAL, 0);
    LD.printString_12x16("  OFF", LCDX2, 6);
    Serial.print("Master OFFline ");
    Serial.println(mb.Hreg(hrSECONDS));
  }
}

void initDS(int i)
{
  sensor[i].init(DS_CONVTIME);
  mb.addHreg(i);                  // Модбас регистр - значение температуры
  mb.addHreg(hrSECONDS + i + 1);  // Модбас регистр - длительность преобразования !DEBUG!
  int pins[] = OWPINS;
  String dsInfo = "ID " + String(i, DEC) + "|Connected " + String(sensor[i].Connected, DEC);
  Serial.println(dsInfo);
  LD.printNumber((long)pins[i]);
  if (i < (sensCount - 1)) LD.printString_6x8(", ");
}

void handleDS(int i)
{
  float t = sensor[i].Temp;
  mb.Hreg(i, round( t * 100));  // заносим в регистр Modbus (температура * 100)
  mb.Hreg((hrSECONDS + i + 1), sensor[i].TimeConv);  // DEBUG!
  String dsInfo = "DS " + String(i, DEC) + ": " + String(t, 2) + " | parasite: " +
                  String(sensor[i].Parasite, DEC) + " | " + String(sensor[i].TimeConv, DEC);
  Serial.println(dsInfo);
  dtostrf(t, 4, 1, cbuf);
  LD.printString_12x16(cbuf, 0, (i * 3));
}

void printFreeRam ()
{
  Serial.print(F("Free RAM: "));
  extern int __heap_start, *__brkval;
  int v, r;
  r = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  Serial.println(r);
}

