
#ifndef _UART_H_
#define _UART_H_

#include "server.h"
#include "wifi.h"


// UART
#define UART_NORMAL         false             // Штатный режим работы UART
#define UART_PROG           true              // UART в режиме программирования MCU
char inChar;                                  // Текущий считанный Байт из буфера UART
char uartData[512] =        "";               // Буфер хранения данных для UART
bool uartMode =             UART_NORMAL;      // Режим работы UART, штатный либо программирования MCU

// СЛУЖЕБНЫЕ СИМВОЛЫ ФОРМИРОВАНИЯ ПАКЕТОВ
#define SOH                 0x01              // Начало пакета
#define US                  0x1F              // Разделитель заголовка и аргумента в пакете
#define STX                 0x02              // Начало данных в пакете
#define ETX                 0x03              // Конец данных в пакете
#define EOT                 0x04              // Конец пакета

// ФУНКЦИИ
void uartInit(int speed);                     // Инициализация UART
void uartWork();                              // Работа с UART, вызывается в основном цикле
void sendUART(const char * header, const char * argument, const char * value); // Отправка пакета в UART


#endif
