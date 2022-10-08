
#include <FS.h>
#include <ArduinoJson.h>
#include "config.h"
extern void sendUART(const char * header, const char * argument, const char * value);

void createLangList();



struct Config configDev;                         // Глобальный объект настроек

// Загрузка параметров из файла config.json
void loadConfig(int target) {
  if (!SPIFFS.exists(CONFIG_PATH) || target == 1) {
    // ЗАГРУЖАЕМ НАСТРОЙКИ ПО УМОЛЧАНИЮ
    strncpy(configDev.di, DEVICE_ID, sizeof(configDev.di) - 1);
    snprintf(configDev.da, sizeof(configDev.da), "%02X", ESP.getChipId());
    strncpy(configDev.rch, "10", sizeof(configDev.rch) - 1);
    strncpy(configDev.dn, configDev.da, sizeof(configDev.dn) - 1);
    strncpy(configDev.dc, "999", sizeof(configDev.dc) - 1);
    strncpy(configDev.dl, "ru", sizeof(configDev.dl) - 1);
    createLangList();
    strncpy(configDev.dv, "0", sizeof(configDev.dv) - 1);
    strncpy(configDev.du, "admin", sizeof(configDev.du) - 1);
    strncpy(configDev.dp, "admin", sizeof(configDev.dp) - 1);
    strncpy(configDev.staSSID, "SOI-GW", sizeof(configDev.staSSID) - 1);
    strncpy(configDev.staPSK, "MCMega20051983!", sizeof(configDev.staPSK) - 1);
    configDev.staIPMode = IP_DHCP;
    strncpy(configDev.staIP, "", sizeof(configDev.staIP) - 1);
    strncpy(configDev.staMS, "", sizeof(configDev.staMS) - 1);
    strncpy(configDev.staGW, "", sizeof(configDev.staGW) - 1);
    snprintf(configDev.apSSID, sizeof(configDev.apSSID), "SOI-%s", configDev.da);
    strncpy(configDev.apPSK, "admin", sizeof(configDev.apPSK) - 1);
    strncpy(configDev.apIP, "192.168.108.1", sizeof(configDev.apIP) - 1);
    strncpy(configDev.apMS, "255.255.255.0", sizeof(configDev.apMS) - 1);
    strncpy(configDev.apGW, "192.168.108.1", sizeof(configDev.apGW) - 1);
    // ... добавить пользовательские переменные
    strncpy(configDev.in1, "In-1", sizeof(configDev.in1) - 1);
    strncpy(configDev.in2, "In-2", sizeof(configDev.in2) - 1);

    return saveConfig();
  }
  File f = SPIFFS.open(CONFIG_PATH, "r");
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, f);
  if (error) {
    sendUART("ER!", "Config", "Loading Settings");
    return;
  }
  JsonObject root = doc.as<JsonObject>();
  if (f) {
    f.close();
  }
  // ЗАГРУЖАЕМ НАСТРОЙКИ ИЗ ФАЙЛА
  strncpy(configDev.di, root["di"], sizeof(configDev.di) - 1);
  strncpy(configDev.da, root["da"], sizeof(configDev.da) - 1);
  strncpy(configDev.rch, root["rch"], sizeof(configDev.rch) - 1);
  strncpy(configDev.dn, root["dn"], sizeof(configDev.dn) - 1);
  strncpy(configDev.dc, root["dc"], sizeof(configDev.dc) - 1);
  strncpy(configDev.dl, root["dl"], sizeof(configDev.dl) - 1);
  createLangList();
  strncpy(configDev.dv, root["dv"], sizeof(configDev.dv) - 1);
  strncpy(configDev.du, root["du"], sizeof(configDev.du) - 1);
  strncpy(configDev.dp, root["dp"], sizeof(configDev.dp) - 1);
  strncpy(configDev.staSSID, root["staSSID"], sizeof(configDev.staSSID) - 1);
  strncpy(configDev.staPSK, root["staPSK"], sizeof(configDev.staPSK) - 1);
  configDev.staIPMode = root["staIPMode"];
  strncpy(configDev.staIP, root["staIP"], sizeof(configDev.staIP) - 1);
  strncpy(configDev.staMS, root["staMS"], sizeof(configDev.staMS) - 1);
  strncpy(configDev.staGW, root["staGW"], sizeof(configDev.staGW) - 1);
  strncpy(configDev.apSSID, root["apSSID"], sizeof(configDev.apSSID) - 1);
  strncpy(configDev.apPSK, root["apPSK"], sizeof(configDev.apPSK) - 1);
  strncpy(configDev.apIP, root["apIP"], sizeof(configDev.apIP) - 1);
  strncpy(configDev.apMS, root["apMS"], sizeof(configDev.apMS) - 1);
  strncpy(configDev.apGW, root["apGW"], sizeof(configDev.apGW) - 1);
  // ... добавить пользовательские переменные
  strncpy(configDev.in1, root["in1"], sizeof(configDev.in1) - 1 );
  strncpy(configDev.in2, root["in2"], sizeof(configDev.in2) - 1 );

  doc.clear();
}

// ********************************************************************** //
// Сохранение параметров в файл config.json
void saveConfig() {
  DynamicJsonDocument doc(2048);
  JsonObject root = doc.to<JsonObject>();
  root["dt"] = DEVICE_TYPE;
  root["dm"] = DEVICE_MODEL;
  root["di"] = configDev.di;
  root["da"] = configDev.da;
  root["rch"] = configDev.rch;
  root["dn"] = configDev.dn;
  root["dc"] = configDev.dc;
  root["dl"] = configDev.dl;
  root["dls"] = configDev.dls;
  root["dv"] = configDev.dv;
  root["du"] = configDev.du;
  root["dp"] = configDev.dp;
  root["staSSID"] = configDev.staSSID;
  root["staPSK"] = configDev.staPSK;
  root["staIPMode"] = configDev.staIPMode;
  root["staIP"] = configDev.staIP;
  root["staMS"] = configDev.staMS;
  root["staGW"] = configDev.staGW;
  root["apSSID"] = configDev.apSSID;
  root["apPSK"] = configDev.apPSK;
  root["apIP"] = configDev.apIP;
  root["apMS"] = configDev.apMS;
  root["apGW"] = configDev.apGW;
  // ... добавить пользовательские переменные
  root["in1"] = configDev.in1;
  root["in2"] = configDev.in2;

  File f = SPIFFS.open(CONFIG_PATH, "w");
  serializeJson(doc, f);
  f.close();
  doc.clear();
}

// ********************************************************************** //
// Создание списка языков на основе сканирования папки dl
void createLangList() {
  DynamicJsonDocument doc(512);
  JsonArray langList = doc.to<JsonArray>();
  Dir dir = SPIFFS.openDir("/dl");
  while (dir.next()) {
    File entry = dir.openFile("r");
    const char *path = entry.name();
    char langName[3] = "";
    int pos = 0, slash_cnt = 0;
    for (int in_pos = 0; in_pos < strlen(path); in_pos++) {
      if (path[in_pos] == '/')
        slash_cnt++;
      else if (slash_cnt == 2) {
        langName[pos] = path[in_pos];
        pos++;
      }
      if (pos > 1)
        break;
    }
    langName[pos] = '\0';
    langList.add(langName);
    entry.close();
  }
  serializeJson(langList, configDev.dls);
}
