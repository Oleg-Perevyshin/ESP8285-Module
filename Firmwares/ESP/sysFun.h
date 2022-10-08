
#ifndef _SYSFUN_H_
#define _SYSFUN_H_

#include "update.h"


// ПЕРЕМЕННЫЕ
String api =                "[[5,9],[2,5,3]]";  // API устройства (для примера)
String sc =                 "[]";             // Сценарии устройства
String settings =           "{\"rs\":\"10\",\"rm\":[2.5,4]}"; // Настройки

// ФУНКЦИИ
void parserSYS(const char * argument, const char * body);
void parserGET(const char * argument);
void parserSET(const char * argument, const char * body);
void parserOK(const char * argument, const char * body);
void parserER(const char * argument, const char * body);
void parsSettings(char * body);


#endif
