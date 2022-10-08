
// Парсер пакетов принятых по UART
// Сюда добавлять команды, которые характерны для отдельных устройств!

#include "sysFun.h"

// ПОЛЬЗОВАТЕЛЬСКИЕ ОБРАБОТЧИКИ UART ПАКЕТОВ
void userParserSYS(const char * argument);
void userParserGET(const char * argument);
void userParserSET(const char * argument, const char * body);
void userParserOK(const char * argument, const char * body);
void userParserER(const char * argument, const char * body);

// ********************************************************************** //
// Протокол https://docs.google.com/document/d/1qBwquRtjmVb4Zq6yYob40aukkdVURFmAt3-TLdhq6yo/edit?usp=sharing
// SYS
void parserSYS(const char *argument, const char * body) {
  if (!strcmp(argument, "BL")) {
    parserBLData(body);                                   // Разбор строки инициализации от BootLoader MCU
  } else if (!strcmp(argument, "checkUpdate")) {
    sendUART("OK!", argument, latVer > VERSION ? "UpdateAvailable" : "NoUpdate");
  } else if (!strcmp(argument, "updStart")) {
    if (latVer > VERSION) {
      sendUART("OK!", argument, "Update...");
      flagUpdateMCU = true;
    } else {
      sendUART("OK!", argument, "NoUpdate");
    }
  } else if (!strcmp(argument, "loadDef")) {
    loadConfig(1);                                        // Загрузка параметров по умолчанию
  } else if (!strcmp(argument, "reStart")) {
    ESP.restart();                                        // Перезагрузка ESP
  } else {
    userParserSYS(argument);
  }
}

// ********************************************************************** //
// GET
void parserGET(const char * argument) {
  if (!strcmp(argument, "ConStatus"))
    sendUART("OK!", "ConStatus", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  else if (!strcmp(argument, "hostName"))
    sendUART("OK!", argument, WiFi.hostname().c_str());
  else if (!strcmp(argument, "netList"))
    WiFi.scanNetworksAsync(printListAPs);
  else if (!strcmp(argument, "ssidSTA"))
    sendUART("OK!", argument, WiFi.SSID().c_str());
  else if (!strcmp(argument, "pskSTA"))
    sendUART("OK!", argument, WiFi.psk().c_str());
  else if (!strcmp(argument, "ipAddr"))
    sendUART("OK!", argument, (WiFi.getMode() == WIFI_STA ? WiFi.localIP() : WiFi.softAPIP()).toString().c_str());
  else if (!strcmp(argument, "devAddr"))
    sendUART("OK!", argument, configDev.da);
  else if (!strcmp(argument, "devID"))
    sendUART("OK!", argument, configDev.di);
  else
    userParserGET(argument);
}

// ********************************************************************** //
// SET
void parserSET(const char * argument, const char * body) {
  if (!strcmp(argument, "devAddr")) {
    strcpy(configDev.da, body);
    saveConfig();
    sendUART("OK!", argument, configDev.da);
  } else if (!strcmp(argument, "staSSID")) {
    strcpy(configDev.staSSID, body);
    saveConfig();
    sendUART("OK!", argument, configDev.staSSID);
  } else if (!strcmp(argument, "staPSK")) {
    strcpy(configDev.staPSK, body);
    saveConfig();
    sendUART("OK!", argument, configDev.staPSK);
  } else
    userParserSET(argument, body);
}

// ********************************************************************** //
// OK!
void parserOK(const char *argument, const char *body) {
  if (!strcmp(argument, "API"))
    api = body;                                                   // Получаем API, пример: [[10,10],[10,10]]
  else if (!strcmp(argument, "SC"))
    sc = body;                                                    // Получаем СЦЕНАРИИ, представляет собой JSON массив объектов, каждый объект - сценарий
  else if (!strcmp(argument, "Settings")) {
    settings = body;                                              // Получаем НАСТРОЙКИ
    sendWS(NULL, "settings", settings);
  }
  else
    userParserOK(argument, body);
}

// ********************************************************************** //
// ER!
void parserER(const char *argument, const char *body) {
  userParserER(argument, body);
}

// ********************************************************************** //
// Парсим ответ от BootLoader (добавить проверку) ␁SYS␟BL␟␂FBF␃71␄ - 64  ␁SYS␟BL␟␂FCF␃B5␄ - 128  ␁SYS␟BL␟␂FDF␃DB␄ - 256  ␁SYS␟BL␟␂FEF␃1F␄ - 512
void parserBLData(const char * body) {
  int deg = (int)body[0] - 0x41;                                  // Вычисляем степень учитывая принятую Букву
  sizeFlashMCU = pow(2, deg) * 1024;                              // Размер Flash MCU в Байтах
  deg = (int)body[1] - 0x3C;                                      // Вычисляем степень учитывая принятую Букву
  sizePackMCU = pow(2, deg);                                      // Размер пакетов для прошивки MCU в Байтах
  deg = (int)body[2] - 0x3C;                                      // Вычисляем степень учитывая принятую Букву
  sizeEEPROMMCU = pow(2, deg);                                    // Размер EEPROM MCU в Байтах

  if (flagUpdate) {                                               // Если запущено обновление, то
    SPIFFS.exists(MCU_FLASH) ? Serial.println(mcuFlash.size()) : Serial.println("0");   // сообщаем загрузчику размер файла Flash,
    SPIFFS.exists(MCU_EEPROM) ? Serial.println(mcuEEPROM.size()) : Serial.println("0"); // размер файла EEPROM,                         
    uartMode = UART_PROG;                                         // устанавливаем UART в режим программирования MCU
    if (mcuFlash.size() == 0 && mcuEEPROM.size() == 0)            // Если размеры обоих файлов 0, то
      updateMCUEnd();                                             // завершаем обновления MCU

  } else {                                                        // Иначе это была перезагрузка MCU
    uartMode = UART_NORMAL;                                       // Переводим UART в штатный режим
    uartFlowCtrl = false;                                         // Запрещаем передачу
  }
}


// ********************************************************************** //
// Парсим ответ SETTINGS и возвращаем в Web
void parsSettings(char * body) {
  //  int i = 0;
  //  char *arr_settings[3];
  //  char *chars_array = strtok(body, ";");
  //  arr_settings[i] = chars_array;
  //  i++;
  //  while (chars_array) {
  //    chars_array = strtok(NULL, ";");
  //    arr_settings[i] = chars_array;
  //    i++;
  //  }
  //  DynamicJsonDocument jsonBuffer;
  //  JsonObject& settings = jsonBuffer.createObject();
  //  settings["gac"] = arr_settings[0];
  //  settings["glt"] = arr_settings[1];
  //  settings["gcs"] = arr_settings[2];
  //  String response;
  //  settings.printTo(response);
  //  ws.textAll(response);
}

void userParserSYS(const char *argument) {
  if (!strcmp(argument, "_test_phrase_")) {
    void();
  } else
    sendUART("ER!", "SYS", "UnknownArgument");
}

void userParserGET(const char *argument) {
  if (!strcmp(argument, "_test_phrase_")) {
    void();
  } else
    sendUART("ER!", "GET", "UnknownArgument");
}

void userParserSET(const char * argument, const char * body) {
  if (!strcmp(argument, "_test_phrase_")) {
    void();
  } else
    sendUART("ER!", "SET", "UnknownArgument");
}

void userParserOK(const char *argument, const char *body) {
  if (!strcmp(argument, "_test_phrase_")) {
    void();
  } else
    sendUART("ER!", "OK!", "UnknownArgument");
}

void userParserER(const char *argument, const char *body) {
  if (!strcmp(argument, "_test_phrase_")) {
    void();
  } else
    sendUART("ER!", "ER!", "UnknownArgument");
}
