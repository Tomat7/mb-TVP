#define IP_NETWORK 192, 168, 1, 0           // сеть в которой будет работать контроллер
#define IP_ADDR_BASE IP_NETWORK + 30        // "начальный" IP-адрес контроллера - к нему прибавляется PLC_ID 
#define ETHERNET_IP IP_ADDR_BASE + PLC_ID   // задаём IP адрес Ethernet модуля 
// то есть при PLC_ID=4 IP-адрес будет 192.168.1.34
// *** более подробное описание смотреть ниже ***

//#define ETHERNET_DHCP     // лучше добавлять в блок конкретного контроллера
// получить IP адрес динамически от DHCP сервера при старте, но скетч занимает больше памяти, особенно с модулем w5100
// если закоментировать, то адрес нужно назначить "принудительно" - необходимо отредактировать IP_NETWORK под свою сеть
// задавать для каждого модуля отдельно! *** посмотреть ETHERNET_IP ***

#if defined(ETHERNET_ENC28J60) || defined(ETHERNET_WIZ5100)

#ifdef ETHERNET_DHCP                        // если не используется DHCP
#define MAC0 0x0A                           // с каким параметром скомпилирован скетч: "0A" - DHCP или "00" - статика
#else
#define MAC0 0x00                           // на всякий случай, чтобы по старшему байту MAC адреса можно было определить
#endif                                      // *** описание смотреть ниже ***

#if defined(ETHERNET_ENC28J60)
#include "EtherCard.h"
#include "ModbusIP_ENC28J60.h"
#define MAC1 0x28               // если второй байт адреса равен 28 - скомпилировано под enc28j60 
#define MAC2 0x60               // на всякий случай, чтобы по второму (после старшего) байту MAC адреса
#elif defined(ETHERNET_WIZ5100) // можно было определить под какой шилд скомпилирован скетч
#include "Ethernet.h"
#include "ModbusIP.h"
#define MAC1 0x51               // если второй байт адреса равен 51 - скомпилировано под wiz5100
#define MAC2 0x00
#endif

#define ETHERNET_MAC MAC0, MAC1, MAC2, 0xEE, 0x30, PLC_ID // MAC адрес Ethernet шилда 
// *** для каждого устройства в сети MAC должен быть уникальным ***
// также смотрите ниже и правьте MAC0-2 и ETHERNET_IP
const byte mac[] = { ETHERNET_MAC };

#endif  // ETHERNET_ENC28J60 || ETHERNET_WIZ5100

#if defined(ESP32) || defined(ESP8266)

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)         // проверялось в режиме умной розетки - SMART_RELAY
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
   Немного про IP-адреса.
   К младшему байту IP-адреса прибавляется PLC_ID, то есть при PLC_ID=5, IP будет 192.168.1.35
   адрес должен быть из диапазона сети в которой планируется использовать модуль, и не должен совпадать
   с адресом любого другого устройства в сети. посмотреть IP адреса сети можно командой ipconfig /all (Windows)
   или ifconfig (Linux) с ПК подключенного и корректно работающего в сети - смотреть на строки
   "IPv4 Address" или "inet addr". но лучше всего залить тестовый скетч для соответствующего модуля
   и посмотреть монитор порта - DHCP должен выдать адрес. это еще и подтвердит работоспособность модуля.
   File->Examples->Ethernet-> DhcpAddressPrinter для w5100
   File->Examples->EtherCard-> testDHCP для enc28j60

   https://ru.wikipedia.org/wiki/%D0%A7%D0%B0%D1%81%D1%82%D0%BD%D1%8B%D0%B9_IP-%D0%B0%D0%B4%D1%80%D0%B5%D1%81
   обычно в домашних сетях используются IP адреса вида:
    10.0.0.1 - 10.255.255.254
    172.16.0.1 - 172.31.255.254
    192.168.0.1 - 192.168.255.254
   вот "что-то подобное" и нужно прописать как IP_ADDR_BASE :) тема бесконечная, пишите - отвечу.

   если не используется DHCP, то кроме адреса можно (иногда и нужно) прописать адрес шлюза по умолчанию
   (default gateway) и адрес(а) DNS сервера (в данном скетче смысла не имеет). шлюз по умолчанию необходим
   если Modbus master (SCADA) и slave (наше устройство) находятся в разных подсетях и между ними маршрутизатор.
   такая конфигурация может возникнуть если модули дома, а SCADA на работе. если понадобится, пишите - расскажу,
   хотя самый простой совет в этом случае использовать DHCP. тема действительно бесконечная, пишите - отвечу.
*/
