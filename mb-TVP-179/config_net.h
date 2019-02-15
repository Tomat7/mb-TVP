#define IP_NETWORK 192, 168, 1, 0           // ���� � ������� ����� �������� ����������
#define IP_ADDR_BASE IP_NETWORK + 30        // "���������" IP-����� ����������� - � ���� ������������ PLC_ID 
#define ETHERNET_IP IP_ADDR_BASE + PLC_ID   // ����� IP ����� Ethernet ������ 
// �� ���� ��� PLC_ID=4 IP-����� ����� 192.168.1.34
// *** ����� ��������� �������� �������� ���� ***

//#define ETHERNET_DHCP     // ����� ��������� � ���� ����������� �����������
// �������� IP ����� ����������� �� DHCP ������� ��� ������, �� ����� �������� ������ ������, �������� � ������� w5100
// ���� ���������������, �� ����� ����� ��������� "�������������" - ���������� ��������������� IP_NETWORK ��� ���� ����
// �������� ��� ������� ������ ��������! *** ���������� ETHERNET_IP ***

#if defined(ETHERNET_ENC28J60) || defined(ETHERNET_WIZ5100)

#ifdef ETHERNET_DHCP                        // ���� �� ������������ DHCP
#define MAC0 0x0A                           // � ����� ���������� ������������� �����: "0A" - DHCP ��� "00" - �������
#else
#define MAC0 0x00                           // �� ������ ������, ����� �� �������� ����� MAC ������ ����� ���� ����������
#endif                                      // *** �������� �������� ���� ***

#if defined(ETHERNET_ENC28J60)
#include "EtherCard.h"
#include "ModbusIP_ENC28J60.h"
#define MAC1 0x28               // ���� ������ ���� ������ ����� 28 - �������������� ��� enc28j60 
#define MAC2 0x60               // �� ������ ������, ����� �� ������� (����� ��������) ����� MAC ������
#elif defined(ETHERNET_WIZ5100) // ����� ���� ���������� ��� ����� ���� ������������� �����
#include "Ethernet.h"
#include "ModbusIP.h"
#define MAC1 0x51               // ���� ������ ���� ������ ����� 51 - �������������� ��� wiz5100
#define MAC2 0x00
#endif

#define ETHERNET_MAC MAC0, MAC1, MAC2, 0xEE, 0x30, PLC_ID // MAC ����� Ethernet ����� 
// *** ��� ������� ���������� � ���� MAC ������ ���� ���������� ***
// ����� �������� ���� � ������� MAC0-2 � ETHERNET_IP
const byte mac[] = { ETHERNET_MAC };

#endif  // ETHERNET_ENC28J60 || ETHERNET_WIZ5100

#if defined(ESP32) || defined(ESP8266)

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)         // ����������� � ������ ����� ������� - SMART_RELAY
#include <ESP8266WiFi.h>
#endif  

#include <ModbusIP_ESP8266.h>
#define WIFI_NAME "Tomat1"
#define WIFI_PASS "filimon7"
IPAddress local_IP(ETHERNET_IP);
IPAddress gateway(IP_NETWORK + 254);
IPAddress subnet(255, 255, 255, 0);
bool wifiConnectOK = false;

#endif  // ESP32 || ESP8266

/*
   ������� ��� IP-������.
   � �������� ����� IP-������ ������������ PLC_ID, �� ���� ��� PLC_ID=5, IP ����� 192.168.1.35
   ����� ������ ���� �� ��������� ���� � ������� ����������� ������������ ������, � �� ������ ���������
   � ������� ������ ������� ���������� � ����. ���������� IP ������ ���� ����� �������� ipconfig /all (Windows)
   ��� ifconfig (Linux) � �� ������������� � ��������� ����������� � ���� - �������� �� ������
   "IPv4 Address" ��� "inet addr". �� ����� ����� ������ �������� ����� ��� ���������������� ������
   � ���������� ������� ����� - DHCP ������ ������ �����. ��� ��� � ���������� ����������������� ������.
   File->Examples->Ethernet-> DhcpAddressPrinter ��� w5100
   File->Examples->EtherCard-> testDHCP ��� enc28j60

   https://ru.wikipedia.org/wiki/%D0%A7%D0%B0%D1%81%D1%82%D0%BD%D1%8B%D0%B9_IP-%D0%B0%D0%B4%D1%80%D0%B5%D1%81
   ������ � �������� ����� ������������ IP ������ ����:
    10.0.0.1 - 10.255.255.254
    172.16.0.1 - 172.31.255.254
    192.168.0.1 - 192.168.255.254
   ��� "���-�� ��������" � ����� ��������� ��� IP_ADDR_BASE :) ���� �����������, ������ - ������.

   ���� �� ������������ DHCP, �� ����� ������ ����� (������ � �����) ��������� ����� ����� �� ���������
   (default gateway) � �����(�) DNS ������� (� ������ ������ ������ �� �����). ���� �� ��������� ���������
   ���� Modbus master (SCADA) � slave (���� ����������) ��������� � ������ �������� � ����� ���� �������������.
   ����� ������������ ����� ���������� ���� ������ ����, � SCADA �� ������. ���� �����������, ������ - ��������,
   ���� ����� ������� ����� � ���� ������ ������������ DHCP. ���� ������������� �����������, ������ - ������.
*/
