#define SKETCHFILE __FILE__ " "
#define SKETCHTIME __DATE__ " " __TIME__
#define SKETCHVERSION SKETCHFILE SKETCHTIME
#define DS_CONVTIME 750   // как часто опрашивать DS18B20 (миллисекунды)
#define MB_TIMEOUT 70     // как долго можно работать/кликать_клапаном без мастера Модбаса (секунды)

#define ETHERNET_MAC MAC0, MAC1, MAC2, 0xEE, 0x30, PLC_ID // MAC адрес Ethernet шилда 
// *** для каждого устройства в сети MAC должен быть уникальным ***
// также смотрите ниже и правьте MAC0-2 и ETHERNET_IP

#ifndef ETHERNET_DHCP                         // если не используется DHCP
#define ETHERNET_IP IP_ADDR_BASE + PLC_ID  // задаём IP адрес Ethernet модуля 
#endif
// *** описание смотреть ниже ***

#ifndef ETHERNET_DHCP     // если компилим _не_ для использования DHCP
#define MAC0 0x00         // на всякий случай, чтобы по старшему байту MAC адреса можно было определить
#else
#define MAC0 0x0A         // с каким параметром скомпилирован скетч: "0A" - DHCP или "00" - статика
#endif

#ifdef ETHERNET_ENC28J60
#include "EtherCard.h"
#include "ModbusIP_ENC28J60.h"
#define MAC1 0x28               // если второй байт адреса равен 28 - скомпилировано под enc28j60 
#define MAC2 0x60               // на всякий случай, чтобы по второму (после старшего) байту MAC адреса
#endif                          // можно было определить под какой шилд скомпилирован скетч

#ifdef ETHERNET_WIZ5100
#include "Ethernet.h"
#include "ModbusIP.h"
#define MAC1 0x51               // если второй байт адреса равен 51 - скомпилировано под wiz5100
#define MAC2 0x00
#endif

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

#include "Modbus.h"
ModbusIP mb;

#include "DStemp.h"
DSThermometer ds18b20[] = DSPINS;

#include "HDvalve.h"
Valve valve[] = VALVEPINS;

#ifdef PRESSURE_BMP
#include "BMP280x.h"
BMP280x bmp280(BMP280_ADDRESS);
#endif

/*
#ifdef PRESSURE_MPX
#include "MPX5010x.h"
MPX5010x mpx5010dp(MPX5010_PIN);
#endif
*/
#ifdef PRESSURE_MPX
#include "ADCmulti.h"
ADCmulti mpx5010dp(MPX5010_PIN);
//int Pins[] = { MPX5010_PIN };
//int Vals[] = { &mpxRAW };
#endif

// ===
#include "ASOLED.h"
#define LCDX1 1           // смещение 1-го "столбца" на экране
#define LCDX2 67          // смещение 2-го "столбца" на экране
// ===
const int nSensor = sizeof(ds18b20) / sizeof(DSThermometer);  // считаем количество DS'ок
const int nValve = sizeof(valve) / sizeof(Valve);             // считаем количество клапанов
// ===
#define hrSECONDS 0                   // регистр-счетчик секунд uptime'а
#define hrTEMP hrSECONDS + 1          // первый регистр с температурой
#define hrPRESSURE hrTEMP + nSensor   // регистр давления
#define hrOPEN hrPRESSURE + 1         // первый регистр с данными для/от клапанов
#define hrCLOSE hrOPEN + 1            // "базовая" скорость - объем собранный за 1000 кликов
#define hrCLICKS hrOPEN + 2           // количество кликов с момента включения
// ===
#ifdef DEBUG_INFO
#define hrDSCONVTIME hrOPEN + nValve*3    // DEBUG!!, таймаут на преобразование DS (можно менять удаленно)   
#define hrDSDEBUG hrDSCONVTIME + 1          // DEBUG!!, будем хранить время преобразования каждой DS'ки
#define hrVALVEDEBUG hrDSDEBUG + nSensor        // DEBUG!!, будем хранить длительность открытия каждого клапана
#endif
// ===

unsigned long msReinit;
uint16_t msGet, msLcd;
uint16_t msTimeout = DS_CONVTIME;
const char degC = 223;
char cbuf[] = {"     "};
bool mbMasterOK = false;
const byte mac[] = { ETHERNET_MAC };

