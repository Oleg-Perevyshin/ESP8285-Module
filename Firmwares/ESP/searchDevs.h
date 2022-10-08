
#ifndef _SEARCHDEVS_H_
#define _SEARCHDEVS_H_

#include <ESPAsyncUDP.h>


// UDP
AsyncUDP udp;                             // Инициализируем UDP
String devsListTemp =   "";               // Временный список устройств в сети
String devsList =       "";               // Итоговый список устройств в сети
String * ipList =       new String [100]; // Массив найденных IP адресов устройств
bool flagHandlePacket = false;            // Чтение пакетов принимаемых по широковещательному запросу

// ФУНКЦИИ
void udpInit();                           // Инициализация UDP
void findDevices();                       // Поиск устройств в сети


#endif
