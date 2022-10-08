
#include "lwip/inet.h"
#include "server.h"

// ********************************************************************** //
// ОБНОВЛЕНИЕ ПРОШИВКИ
// ЗАБИРАЕМ ДАННЫЕ С СЕРВЕРА OTA_HOST ПО ПУТИ path И ОБРАБАТЫВАЕМ (асинхронно, HTTP)
void getServerData(const char * path) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!strcmp(path, OTA_PATH)) {                    // Если запрошено обновление ESP
      flagCheckUpdate = false;                        // Сбрасываем флаг
    } else if (!strcmp(path, OTA_PATH_FLASH)) {       // Если запрошено обновление MCU
      flagGetMCUF = false;                            // Сбрасываем флаг скачивания
      if (SPIFFS.exists(MCU_FLASH)) {                 // Если есть старые файлы прошивки
        if (mcuFlash)                                 // проверяем, открыт ли mcuFlash, если открыт, то
          mcuFlash.close();                           // закрываем
        SPIFFS.remove(MCU_FLASH);                     // и удаляем mcuFlash
      }
    } else if (!strcmp(path, OTA_PATH_EEPROM)) {
      flagGetMCUE = false;
      if (SPIFFS.exists(MCU_EEPROM)) {                // Если есть старые файлы прошивки
        if (mcuEEPROM)                                // проверяем, открыт ли mcuEEPROM, если открыт, то
          mcuEEPROM.close();                          // закрываем
        SPIFFS.remove(MCU_EEPROM);                    // и удаляем mcuEEPROM
      }
    } else
      return;                                         // Если путь не передан - выходим

    if (aClient)
      return;                                         // Если клиент создан - выходим
    else
      aClient = new AsyncClient();                    // Если клиента нет - создаем

    if (!aClient)
      return;                                         // Если не уалось создать - выходим

    aClient->onError([](void * arg, AsyncClient * client, int error) {
      aClient = NULL;
      delete client;
    }, NULL);

    aClient->onConnect([ = ](void * arg, AsyncClient * client) {
      if (!strcmp(path, OTA_PATH_FLASH))
        mcuFlash = SPIFFS.open(MCU_FLASH, "w");
      if (!strcmp(path, OTA_PATH_EEPROM))
        mcuEEPROM = SPIFFS.open(MCU_EEPROM, "w");
      aClient->onError(NULL, NULL);
      client->onDisconnect([ = ](void * arg, AsyncClient * c) {   // Соединение закрыто (получили данные)
        if (!strcmp(path, OTA_PATH)) {                            // Прочитали id.txt и сейчас
          flagGetMCUF = true;                                     // запускаем скачивание прошивки MCU (Flash)
        } else if (!strcmp(path, OTA_PATH_FLASH)) {               // Если скачана прошивка MCU (Flash), то
          flagGetMCUE = true;                                     // запускаем скачивание прошивки MCU (EEPROM)
        } else if (!strcmp(path, OTA_PATH_EEPROM)) {              // Если скачана прошивка MCU (EEPROM), то
          flagCheckBINs = true;                                   // запускаем проверку md5
        }
        aClient = NULL;
        delete c;
      }, NULL);
      gotHeaders = false;
      client->onData([ = ](void * arg, AsyncClient * c, void * data, size_t len) {
        uint8_t * d = (uint8_t*)data;
        if (!gotHeaders) {                          // Пропуск заголовка ответеа сервера
          gotHeaders = true;
          d = (uint8_t *)(strstr((char*)data, "\r\n\r\n") + 4);
          len -= d - (uint8_t*)data;
        }
        if (!strcmp(path, OTA_PATH)) {              // Если передан путь обновления ESP, то
          const char * response = (char*)d;         // читаем ответ (без заголовка)
          const char * _md5FlashLatest = strchr(response, ':') + 1; // Указатель на начало контрольной суммы прошивки MCU (Flash)
          const char * _md5EEPROMLatest = strchr(_md5FlashLatest, '-') + 1;  // Указатель на начало контрольной суммы прошивки MCU (EEPROM)
          const char * _md5ffsLatest = strchr(_md5EEPROMLatest, '-') + 1;  // Указатель на начало контрольной суммы прошивки FFS
          const char * _md5espLatest = strchr(_md5ffsLatest, '-') + 1;  // Указатель на начало контрольной суммы прошивки ESP
          latVer = strtol(response, NULL, 10);      // получаем версию прошивки на сервере
          strncpy(md5FlashLatest, _md5FlashLatest, 32); md5FlashLatest[32] = '\0'; // получаем контрольную сумму прошивки MCU (Flash) и закрываем строку
          strncpy(md5EEPROMLatest, _md5EEPROMLatest, 32); md5EEPROMLatest[32] = '\0'; // получаем контрольную сумму прошивки MCU (EEPROM) и закрываем строку
          strncpy(md5ffsLatest, _md5ffsLatest, 32); md5ffsLatest[32] = '\0'; // получаем контрольную сумму прошивки FFS и закрываем строку
          strncpy(md5espLatest, _md5espLatest, 32); md5espLatest[32] = '\0'; // получаем контрольную сумму прошивки ESP и закрываем строку
        } else if (!strcmp(path, OTA_PATH_FLASH)) { // Если передан путь обновления MCU (Flash), то
          mcuFlash.write(d, len);                   // скачиваем прошивку в файловую систему
        } else if (!strcmp(path, OTA_PATH_EEPROM)) {// Если передан путь обновления MCU (EEPROM), то
          mcuEEPROM.write(d, len);                  // скачиваем прошивку в файловую систему
        } else {                                    // В ином случае
          aClient = NULL;                           // сбрасываем клиента
          delete c;                                 // и удаляем
        }
      }, NULL);
      char request[128];                            // Буфер для запроса на сервер
      sprintf(request,
              "GET %s HTTP/1.1\r\n"
              "Host: %s\r\n"
              "User-Agent: %s\r\nConnection: close\r\n\r\n",
              path, OTA_HOST, DEVICE_MODEL
             );
      client->write(request);                   // Передаем запрос на сервер
    }, NULL);

    if (!aClient->connect(OTA_HOST, 80)) {
      AsyncClient * client = aClient;
      aClient = NULL;
      delete client;
    }
  }
}

// ********************************************************************** //
// Закачка файла прошивки MCU //
void mcuFlashUpload(AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final) {
  if (!index) {
    if (mcuFlash)
      mcuFlash.close();
    SPIFFS.remove(MCU_FLASH);
    mcuFlash = SPIFFS.open(MCU_FLASH, "w");
  }
  for (size_t i = 0; i < len; i++) {
    char(mcuFlash.write(data[i]));
  }
  if (final) {
    mcuFlash.flush();
    mcuFlash.close();
  }
}

// ********************************************************************** //
// Закачка файла прошивки MCU EEPROM //
void mcuEEPROMUpload(AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final) {
  if (!index) {
    if (mcuEEPROM)
      mcuEEPROM.close();
    SPIFFS.remove(MCU_EEPROM);
    mcuEEPROM = SPIFFS.open(MCU_EEPROM, "w");
  }
  for (size_t i = 0; i < len; i++) {
    char(mcuEEPROM.write(data[i]));
  }
  if (final) {
    mcuEEPROM.flush();
    mcuEEPROM.close();
  }
}

// ********************************************************************** //
void serverInit() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);       // Асинхронный WebSocket

  server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest * request) {
    StreamString output;
    if (output.reserve(1024)) {
      if (!WiFi.getMode() == WIFI_STA)
        return;
      uint32_t ip = WiFi.localIP();
      uint32_t chipId = ESP.getChipId();
      output.printf(
        ssdpTemplate,
//        IP2STR(&ip),                        // URLBase            (IP адрес устройства)
        DEVICE_MODEL, configDev.dn,         // friendlyName       (SOI-WiFi | Пользовательское имя)
        "SOI Tech",                         // manufacturer
        "http://soi-tech.com",              // manufacturerURL
        DEVICE_MODEL,                       // modelName          (SOI-WiFi)
        DEVICE_ID,                          // modelNumber        (F9)
        DEVICE_URL,                         // modelURL           (http://soi-tech.com/production/F9)
        configDev.da,                       // serialNumber       (Адрес устройства в Радио сети)
        (uint8_t) ((chipId >> 16) & 0xff),  // UDN
        (uint8_t) ((chipId >>  8) & 0xff),  // UDN
        (uint8_t)   chipId        & 0xff    // UDN EDC3B25E-C2D0-411E-B04B-F9BD5E%02x%02x%02x
      );
      request->send(200, "text/xml; charset=utf-8", (String)output);
    } else
      request->send(500);
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(configDev.du, configDev.dp))
      return request->requestAuthentication();
    AsyncWebServerResponse *response;
    response = request->beginResponse_P(200, "text/html; charset=utf-8", index_htm);
    response->addHeader("Server", "soi-tech");
    request->send(response);
  });

  //  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
  //    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css; charset=utf-8", style_css_gz, style_css_gz_len);
  //    response->addHeader("Content-Encoding", "gzip");
  //    request->send(response);
  //  });

  //  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest * request) {
  //    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript; charset=utf-8", main_js_gz, main_js_gz_len);
  //    response->addHeader("Content-Encoding", "gzip");
  //    request->send(response);
  //  });

  server.on("/icon", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/svg+xml; charset=utf-8", icon);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  server.on("/dev", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8", dev);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // ВЕРНУТЬ ДАННЫЕ КОНФИГУПАЦИИ УСТРОЙСТВА
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, CONFIG_PATH, "text/json; charset=utf-8");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // УСТАНОВИТЬ ЯЗЫК ИНТЕРФЕЙСА И ВЕРНУТЬ СЛОВАРЬ
  server.on("/lang", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response;
    if (!request->hasArg("get") || !request->hasArg("set")) {
      return request->send(400, "text/plain; charset=utf-8", "Bad Request");
    }
    strncpy(configDev.dl, request->arg("get").c_str(), sizeof(configDev.dl) - 1);
    if (request->arg("set").equals("1")) {
      saveConfig();
    }
    String path = "/dl/"; path += configDev.dl; path += ".json";
    response = request->beginResponse(SPIFFS, path, "text/json; charset=utf-8");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // ПОЛУЧИТЬ API устройства
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/json; charset=utf-8", api);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // ПОЛУЧИТЬ СЦЕНАРИИ
  server.on("/scr", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/json; charset=utf-8", sc);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });

  // Загрузка файла прошивки MCU в SPIFFS
  server.on("/mcuFlash", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", "OK");
  }, mcuFlashUpload);

  // Загрузка файла прошивки MCU в SPIFFS
  server.on("/mcuEEPROM", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain; charset=utf-8", "OK");
  }, mcuEEPROMUpload);

  server.onNotFound([](AsyncWebServerRequest * request) {
    if (request->method() == HTTP_OPTIONS)
      request->send(200, "text/plain; charset=utf-8", "OK");
    else
      request->send(404, "text/plain; charset=utf-8", "Not Found");
  });
  server.serveStatic("/", SPIFFS, "/");   // Доступ к файловой системе
  server.begin();
}

// ОБРАБОТЧИК СОБЫТИЙ WEBSOCKET
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t * data, size_t len) {
  if (type == WS_EVT_CONNECT) {                                                      // КЛИЕНТ ПОДКЛЮЧЕН
    client->ping();                                                                  // Поддержание соединения (если закомм. предыдущий сокет отключается)
    sendWS(NULL, "infoMessage", "WS - Connect");
  } else if (type == WS_EVT_DISCONNECT) {                                            // КЛИЕНТ ОТКЛЮЧЕН
    void();
  } else if (type == WS_EVT_ERROR) {                                                 // ОБРАБОТКА ОШИБОК
    void();
  } else if (type == WS_EVT_PONG) {                                                  // ПОЛУЧЕНО СООБЩЕНИЕ PONG (возможно, в ответ на запрос ping)
    void();
  } else if (type == WS_EVT_DATA) {                                                  // ОБРАБОТКА ВХОДЯЩИХ ПАКЕТОВ WEBSOCKET
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, (char*)data);                  // Парсим пакет
    if (error) {                                                                     // если ошибка в пакете
      sendUART("ER!", "WS-Data", "Parser");                                          // выводим в UART
      return;                                                                        // и выходим
    }
    JsonObject root = doc.as<JsonObject>();                                          // Создаем объект root
    if (root.containsKey("ActualInfo")) {                                            // АКТУАЛЬНЫЕ ПАРАМЕТРЫ УСТРОЙСТВА
      DynamicJsonDocument doc(4096);
      JsonObject json = doc.to<JsonObject>();
      // Общие параметры
      json["actDM"] = DEVICE_MODEL; json["actDN"] = configDev.dn;
      json["actID"] = DEVICE_ID;
      json["actV"] = VERSION; json["actLV"] = latVer;
      json["actDU"] = configDev.du;
      json["actDP"] = configDev.dp;
      // Параметры "STA"
      json["actWCH"] = WiFi.channel();
      json["actWRSSI"] = WiFi.RSSI();
      json["actStaSSID"] = WiFi.SSID();
      json["actStaPSK"] = WiFi.psk();
      json["actStaIP"] = WiFi.localIP().toString();
      json["actStaMS"] = WiFi.subnetMask().toString();
      json["actStaGW"] = WiFi.gatewayIP().toString();
      json["actStaMAC"] = WiFi.macAddress();
      // Параметры "AP"
      json["actApSSID"] = configDev.apSSID;
      json["actApPSK"] = configDev.apPSK;
      json["actApIP"] = configDev.apIP;
      json["actApMS"] = configDev.apMS;
      json["actApGW"] = configDev.apGW;
      json["actApMAC"] = WiFi.softAPmacAddress();
      // Параметры Radio
      json["actDA"] = configDev.da;
      json["actRCH"] = configDev.rch;
      String actualInfo;
      serializeJson(json, actualInfo);
      sendWS(NULL, "ActualInfo", actualInfo);
    } else if (root.containsKey("Settings")) {
      sendWS(NULL, "Settings", settings);
    } else if (root.containsKey("NetList")) {                                        // СПИСОК ДОСТУПНЫХ ТОЧЕК ДОСТУПА
      WiFi.scanNetworksAsync(printListAPs);
    } else if (root.containsKey("DevScan")) {                                        // ЗАПУСК СКАНИРОВАНИЯ СЕТИ
      findDevices();
    } else if (root.containsKey("DevsList")) {                                       // СПИСОК УСТРОЙСТВ В СЕТИ
      sendWS(NULL, "DevsList", devsList);
    } else if (root.containsKey("SetID")) {                                          // УСТАНОВИТЬ ПАРАМЕТРЫ ИДЕНТИФИКАЦИИ , dc, dn, du, dp
      strncpy(configDev.dc, root["SetID"][0], sizeof(configDev.dc) - 1);
      strncpy(configDev.dn, root["SetID"][1], sizeof(configDev.dn) - 1);
      strncpy(configDev.du, root["SetID"][2], sizeof(configDev.du) - 1);
      strncpy(configDev.dp, root["SetID"][3], sizeof(configDev.dp) - 1);
      saveConfig();
    } else if (root.containsKey("SetSTA")) {                                          // УСТАНОВИТЬ ПАРАМЕТРЫ ПОДКЛЮЧЕНИЯ К СЕТИ staSSID, staPSK, staIPMode, staIP, staMS, staGW
      strncpy(configDev.staSSID, root["SetSTA"][0], sizeof(configDev.staSSID) - 1);   // Читаем SSID выбранной сети
      strncpy(configDev.staPSK, root["SetSTA"][1], sizeof(configDev.staPSK) - 1);     // Читаем пароль
      configDev.staIPMode = root["SetSTA"][2].as<int>();                              // Читаем режим сетевых настроек (0-DHCP, 1-Static)
      if (configDev.staIPMode == IP_STATIC) {                                         // Если заданы статические параметры сети - проверяем значения (если адреса не валидны - устанавливаем текущее значение)
        // Валидация адресов (приводим к стандартному виду, если исходные данные невалидны, устанавливаем текущие значения, полученные от DHCP)
        handleIP(root["SetSTA"][3]) == "" ? strncpy(configDev.staIP, WiFi.localIP().toString().c_str(), sizeof(configDev.staIP) - 1) : strncpy(configDev.staIP, handleIP(root["SetSTA"][3]).c_str(), sizeof(configDev.staIP) - 1);
        handleIP(root["SetSTA"][4]) == "" ? strncpy(configDev.staMS, WiFi.subnetMask().toString().c_str(), sizeof(configDev.staMS) - 1) : strncpy(configDev.staMS, handleIP(root["SetSTA"][4]).c_str(), sizeof(configDev.staMS) - 1);
        handleIP(root["SetSTA"][5]) == "" ? strncpy(configDev.staGW, WiFi.gatewayIP().toString().c_str(), sizeof(configDev.staGW) - 1) : strncpy(configDev.staGW, handleIP(root["SetSTA"][5]).c_str(), sizeof(configDev.staGW) - 1);
      } else {                                                                        // Если заданы динамические параметры сети, то
        strncpy(configDev.staIP, "", sizeof(configDev.staIP) - 1);                    // очищаем значение IP адреса
        strncpy(configDev.staMS, "", sizeof(configDev.staMS) - 1);                    // маски подсети
        strncpy(configDev.staGW, "", sizeof(configDev.staGW) - 1);                    // и IP адреса шлюза
      }
      saveConfig();                                                                   // Сохраняем параметры модуля для режима STA
    } else if (root.containsKey("SetRF")) {                                           // УСТАНОВИТЬ ПАРАМЕТРЫ РАДИО
      if (root["SetRF"][0] == "")                                                     // Если Радио адрес не задан, то
        snprintf(configDev.da, sizeof(configDev.da), "%02X", ESP.getChipId());        // устанавливаем его по умолчанию (3 последних байта MAC адреса - 6 символов)
      else                                                                            // Иначе
        strncpy(configDev.da, root["SetRF"][0], sizeof(configDev.da) - 1);            // читаем принятое значение
      strncpy(configDev.rch, root["SetRF"][1], sizeof(configDev.rch) - 1);            // Читаем частотный радиоканал
      char rfData[sizeof(configDev.da) + sizeof(configDev.rch) + 1];                  // Собираем
      snprintf(rfData, sizeof(rfData), "%s;%s", configDev.da, configDev.rch);         // пакет для отпраки
      sendUART("SET", "RF", rfData);                                                  // Передаем параметры в MCU
      saveConfig();                                                                   // Сохраняем параметры в config.json
    } else if (root.containsKey("SetAP")) {                                           // УСТАНОВИТЬ ПАРАМЕТРЫ УСТРОЙСТВА В РЕЖИМЕ ТОЧКИ ДОСТУПА apSSID, apPSK, apIP, apMS, apGW
      if (root["SetAP"][0] == "")                                                     // Если SSID в режиме AP не задан, то
        snprintf(configDev.apSSID, sizeof(configDev.apSSID), "SOI-%s", configDev.da); // устанавливаем его по умолчанию "SOI-deviceAddress"
      else                                                                            // Иначе
        strncpy(configDev.apSSID, root["SetAP"][0], sizeof(configDev.apSSID) - 1);    // читаем принятое значение
      strncpy(configDev.apPSK, root["SetAP"][1], sizeof(configDev.apPSK) - 1);        // Читаем пароль к устройству в режиме "Точка доступа"
      // Валидация адресов (приводим к стандартному виду, если исходные данные невалидны, устанавливаем значение по умолчанию)
      handleIP(root["SetAP"][2]) == "" ? strncpy(configDev.apIP, "192.168.108.1", sizeof(configDev.apIP) - 1) : strncpy(configDev.apIP, handleIP(root["SetAP"][2]).c_str(), sizeof(configDev.apIP) - 1);
      handleIP(root["SetAP"][3]) == "" ? strncpy(configDev.apMS, "255.255.255.0", sizeof(configDev.apMS) - 1) : strncpy(configDev.apMS, handleIP(root["SetAP"][3]).c_str(), sizeof(configDev.apMS) - 1);
      handleIP(root["SetAP"][4]) == "" ? strncpy(configDev.apGW, "192.168.108.1", sizeof(configDev.apGW) - 1) : strncpy(configDev.apGW, handleIP(root["SetAP"][4]).c_str(), sizeof(configDev.apGW) - 1);
      saveConfig();                                                                   // Сохраняем параметры модуля для режима AP
    } else if (root.containsKey("SetSC")) {                                           // ПОЛУЧЕНИЕ СЦЕНАРИЕВ из WS
      //      const char * sc = root["setSC"];
      //      sendUART("SET", "SC", sc);
      String toUART = "";
      serializeJson(root["SetSC"], toUART);
      sendUART("SET", "SC", toUART.c_str());                                          // Передача их в MCU
    } else if (root.containsKey("Synch")) {                                           // ПЕРЕДАЧА ПАРАМЕТРОВ АВТОРИЗАЦИИ (staSSID, staPSK - для рассылки по радио сети)
      char synchData[sizeof(configDev.staSSID) + sizeof(configDev.staPSK) + 1];
      snprintf(synchData, sizeof(synchData), "%s;%s", configDev.staSSID, configDev.staPSK);
      sendUART("SET", "Synch", synchData);
    } else if (root.containsKey("DV")) {                                              // УСТАНОВКА ВИЗУАЛЬНОЙ ТЕМЫ
      strncpy(configDev.dv, root["DV"][0], sizeof(configDev.dv) - 1);
      saveConfig();
    } else if (root.containsKey("IOs")) {                                             // УСТАНОВКА НАЗВАНИЙ КАНАЛОВ
      strncpy(configDev.in1, root["IOs"][0], sizeof(configDev.in1) - 1);
      strncpy(configDev.in2, root["IOs"][1], sizeof(configDev.in2) - 1);
      saveConfig();
    } else if (root.containsKey("UpdCheck")) {                                        // ПРОВЕРКА НАЛИЧИЯ ОБНОВЛЕНИЯ
      latVer > VERSION ? sendWS(NULL, "UpdCheck", "true") : sendWS(NULL, "UpdCheck", "false");
    } else if (root.containsKey("UpdStart")) {                                        // ЗАПУСК ОБНОВЛЕНИЯ
      if (root["UpdStart"][0] == "esp") {
        flagUpdateMCU = false;
        flagUpdateESP = true;
        flagUpdateALL = false;
      } else if (root["UpdStart"][0] == "mcu") {
        flagUpdateMCU = true;
        flagUpdateESP = false;
        flagUpdateALL = false;
      } else if (root["UpdStart"][0] == "all") {
        flagUpdateMCU = true;
        flagUpdateESP = false;
        flagUpdateALL = true;
      } else {                                                                        // ОБНОВЛЕНИЕ ПРОШИВКИ ПО ID
        unsigned int id;
        sscanf(root["UpdStart"][0], "%x", &id);
        if (id >= 0 && id <= 255) {                                                     // Проверить диапазон 0-255
          strncpy(configDev.di, root["UpdStart"][0], sizeof(configDev.di) - 1);       // Получаем ID
          sprintf(OTA_PATH, "/Updates/%s.txt", configDev.di);                         // Обновляем путь к файлу ID.txt на сервере
          sprintf(OTA_PATH_FLASH, "/Updates/%s-MF.bin", configDev.di);                // Обновляем путь к файлу BIN на сервере
          sprintf(OTA_PATH_EEPROM, "/Updates/%s-ME.bin", configDev.di);               // Обновляем путь к файлу BIN на сервере
          flagCheckUpdate = true;                                                     // Запускаем проверку обновлений на сервере
          flagUpdate = true;                                                          // Устанавливаем флаг запуска обновления
          flagUpdateALL = true;                                                       // и флаг обновления всего устройства
          sendWS(NULL, "infoMessage", "Update by ID");
          sendUART("OK!", "Update by ID", NULL);
        } else {
          flagCheckUpdate = false;                                                    // Запускаем проверку обновлений на сервере
          flagUpdate = false;                                                         // Устанавливаем флаг запуска обновления
          flagUpdateALL = false;                                                      // и флаг обновления всего устройства
          sendWS(NULL, "infoMessage", "Unknown ID");
          sendUART("ER!", "Unknown ID", NULL);
        }
      }
    } else if (root.containsKey("UpdClear")) {                                        // УДАЛЕНИЕ ФАЙЛОВ ПРОШИВОК
      if (SPIFFS.exists(MCU_FLASH)) {                                                 // Если есть старый файла прошивки по пути MCU_FLASH
        if (mcuFlash)                                                                 // проверяем, открыт ли он, если открыт, то
          mcuFlash.close();                                                           // закрываем
        SPIFFS.remove(MCU_FLASH);                                                     // и удаляем
      }
      if (SPIFFS.exists(MCU_EEPROM)) {                                                // Если есть старый файла прошивки по пути MCU_FLASH
        if (mcuEEPROM)                                                                // проверяем, открыт ли он, если открыт, то
          mcuEEPROM.close();                                                          // закрываем
        SPIFFS.remove(MCU_EEPROM);                                                    // и удаляем
      }
    } else if (root.containsKey("Rst")) {                                             // ПЕРЕЗАГРУЗКА УСТРОЙСТВА
      sendUART("SYS", "RST", "all");
      ESP.restart();
    } else if (root.containsKey("DefSettings")) {                                     // СБРОС НАСТРОЕК ПО УМОЛЧАНИЮ
      if (SPIFFS.remove(CONFIG_PATH)) {
        loadConfig(1);
      }
      sendUART("SYS", "DefSettings", "all");
    } else if (root.containsKey("mcu")) {                                             // ПЕРЕДАЧА КЛЮЧ / ЗНАЧЕНИЯ В MCU ␁SET␟key␟␂val␃CRC8␄
      sendUART("SET", root["mcu"][0], root["mcu"][1]);
    } else {                                                                          // НЕВЕРНЫЕ ДАННЫЕ (НЕТ ЗАГОЛОВКА ПАКЕТА)
      sendUART("ER!", "WS", "Unknown Data");
      sendWS(NULL, "infoMessage", "WS - Unknown Data");
    }
  }
}

// ********************************************************************** //
// Валидация и трансформация IP адреса
// Приводит адрес к формату 0-255.0-255.0-255.0-255, если исходные данные не верные - возвращает пустую строку
String handleIP (String ip) {
  int a[4];
  const char *strIn = ip.c_str();
  String strOut = "";
  sscanf(strIn, "%d.%d.%d.%d", a + 0, a + 1, a + 2, a + 3);
  for (int i = 0; i < 4; i++) {
    if (a[i] < 0 || a[i] > 255) {
      strOut = "";
      return strOut;
    } else {
      if (i == 3) {
        strOut += a[i];
      } else {
        strOut += a[i];
        strOut += '.';
      }
    }
  }
  return strOut;
}

// ********************************************************************** //
// ПЕРЕДАЧА ДАННЫХ В WEBSOCKET
// Передаем объект: { argument: data }
void sendWS(AsyncWebSocketClient *client, const char *argument, String value) {
  DynamicJsonDocument doc(2048);
  JsonObject root = doc.to<JsonObject>();
  root[argument] = value;
  size_t len = measureJson(doc);
  AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
  if (buffer) {
    serializeJson(root, (char*)buffer->get(), len + 1);
    ws.textAll(buffer);
  }
}
