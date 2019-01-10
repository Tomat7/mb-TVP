
void initETH()
{
#ifdef ESP32
  if (!WiFi.config(local_IP, gateway, subnet)) Serial.println("ESP32 WiFi failed to configure");
  Serial.print("Connecting to ");
  Serial.println(WIFI_NAME);
  WiFi.begin(WIFI_NAME, WIFI_PASS);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  static char macbuff[17] = { "                " };
  String macStr = WiFi.macAddress();
  macStr.toCharArray(macbuff, 18);
  LD.printString_6x8("MAC:", LCDX1, 1);
  LD.printString_6x8(macbuff, LCDX1 + 25, 1);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  mb.slave();
#endif
}

void serialIP()
{
#ifdef ESP32
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
#endif
}

void displayIP()
{
#ifdef ESP32
  static char macbuff[17] = { "                " };
  sprintf(macbuff, "%i.%i.%i.%i", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  LD.printString_6x8("IP:", LCDX1, 3);
  LD.printString_6x8(macbuff, LCDX1 + 22, 3);
#endif

}
