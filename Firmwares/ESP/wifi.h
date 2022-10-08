
#ifndef _WIFI_H_
#define _WIFI_H_

#include "uart.h"

// ФУНКЦИИ
void wifiInit();                          // Инициализация модуля (подключается к AP при возможности, иначе вызывает startAPSTA)
void wifiPrintStatus();                   // Выводит данные статуса в UART (TODO доделать нужный перечень выводимых параметров)
void checkConnect();                      // Проверка наличия подлючения к сети
void printListAPs(int networksFound);     // Возвращает список найденных сетей


#endif
