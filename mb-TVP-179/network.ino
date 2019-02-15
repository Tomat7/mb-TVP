void initETH()
{
#if defined(ETHERNET_ENC28J60) || defined(ETHERNET_WIZ5100)

#ifdef ETHERNET_DHCP
    mb.config(mac);
#else
    const byte ip[] = { ETHERNET_IP };
    mb.config(mac, ip);
#endif

#elif defined(ESP32) || defined(ESP8266)

	//WiFi.mode(WIFI_STA);
	wifiConnectOK = false;
	mbMasterOK = false;
	if (!WiFi.config(local_IP, gateway, subnet)) CFG_PRINTLN("ESP32 WiFi failed to configure");
    CFG_PRINT("Connecting to ");
    CFG_PRINTLN(WIFI_NAME);
    WiFi.begin(WIFI_NAME, WIFI_PASS);
    CFG_PRINT("MAC: ");
    CFG_PRINTLN(WiFi.macAddress());

    static char macbuff[17] = { "                " };
    String macStr = WiFi.macAddress();
    macStr.toCharArray(macbuff, 18);
    LD_printString_6x8("MAC:", LCDX1, 1);
    LD_printString_6x8(macbuff, LCDX1 + 25, 1);

#ifdef LED_INFO
    valve[LED_INFO].setTime(100, 100);
#endif

	while (!wifiConnectOK) { checkWiFi(); delayMs(100); }
    mb.slave();

#ifdef LED_INFO
	valve[LED_INFO].setTime(1500, 100);
	modbusON = true;
#endif

#endif
}

void checkWiFi()
{
#if defined(ESP32) || defined(ESP8266)
    if (WiFi.status() == WL_CONNECTED) wifiConnectOK = true;
    else { PRINT("."); wifiConnectOK = false;  }
#endif
}

void serialMAC()
{
#ifdef SERIAL_CONFIG
#if defined(ETHERNET_ENC28J60) || defined(ETHERNET_WIZ5100)
    Serial.print("MAC: ");
    for (byte i = 0; i < 6; ++i)
    {
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(':');
    }
    Serial.println();
#endif
#endif
}

void displayMAC()
{
#if defined(ETHERNET_ENC28J60) || defined(ETHERNET_WIZ5100)
    LD_printString_6x8("MAC: ", LCDX1, 1);
    for (byte i = 0; i < 6; ++i)
    {
        char mh[2];
        String macStr = String(mac[i], HEX);
        macStr.toUpperCase();
        macStr.toCharArray(mh, 3);
        LD_printChar_6x8(mh);
        if (i < 5) LD_printChar_6x8(":");
    }
#endif
}

void serialIP()
{
#ifdef SERIAL_CONFIG
#if defined(ETHERNET_ENC28J60)
    ether.printIp("NC28J60 IP: ", ether.myip);
    ether.printIp("MASK: ", ether.netmask);
    ether.printIp("GW: ", ether.gwip);
    //ether.printIp("DNS: ", ether.dnsip);
#elif (ETHERNET_WIZ5100)
    Serial.print(F("WIZ5100 IP: "));
    Serial.println(Ethernet.localIP());
#elif defined(ESP32) || defined(ESP8266)
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
#endif
#endif
}

void displayIP()
{
#if defined(ETHERNET_ENC28J60)
    LD_printString_6x8("enc_IP: ", LCDX1, 3);
    for (byte i = 0; i < 4; ++i)
    {
        LD_printNumber((long)ether.myip[i]);
        if (i < 3) LD_printChar_6x8(".");
    }
#elif defined(ETHERNET_WIZ5100)
    LD_printString_6x8("wiz_ip: ", LCDX1, 3);
    for (byte i = 0; i < 4; ++i)
    {
        LD_printNumber((long)Ethernet.localIP()[i]);
        if (i < 3) LD_printChar_6x8(".");
    }
#elif defined(ESP32) || defined(ESP8266)
    static char macbuff[17] = { "                " };
    sprintf(macbuff, "%i.%i.%i.%i", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
    LD_printString_6x8("IP:", LCDX1, 3);
    LD_printString_6x8(macbuff, LCDX1 + 22, 3);
#endif
}
