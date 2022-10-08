
#include "wifi.h"
#include "config.h"

// ********************************************************************** //
// ИНИЦИАЛИЗАЦИЯ WIFI
void wifiInit() {
  digitalWrite(STATUS_LED_PIN, HIGH);               // Начальное состояние 1
  WiFi.mode(WIFI_OFF);
  if (startSTA()) {
    digitalWrite(STATUS_LED_PIN, LOW);
    ledTicker.detach();                             // Выключение моргания светодиодом - признак режима STA
    flagCheckUpdate = true;                         // Запуск проверки наличия обновления на сервере
    udpInit();                                      // Инициализация UDP
    WiFi.setAutoReconnect(true);                    // Активация переподключения при потере связи
    sendUART("SYS", "ConStatus", "Connected");
  } else {
    startAPSTA();                                   // Запуск режима AP+STA
    ledTicker.attach(0.5, changeLedState);          // Запуск моргания светодиодом - признак режима AP+STA
    sendUART("SYS", "ConStatus", "Disconnected");
  }
  WiFi.setAutoConnect(true);                        // Автоматическое подключение к последней точке доступа при вкл. питания
  findDevices();                                    // Запуск поиска устройств в сети
  WiFi.hostname(configDev.apSSID);                  // Указание имени хоста
  wifiPrintStatus();                                // Вывод статуса в UART
}

// ********************************************************************** //
// ЗАПУСК РЕЖИМА STA
bool startSTA() {
  WiFi.mode(WIFI_STA);                              // Установка режима STA
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);               // Устанавливаем режим WiFi
  WiFi.begin(configDev.staSSID, configDev.staPSK);  // Указание параметров авторизации
  if (configDev.staIPMode == IP_STATIC) {           // Если заданы статические параметры, то
    IPAddress address, netmask, gateway;            // созаем переменные и заносим значения
    address.fromString(configDev.staIP);            // IP адрес модуля в режиме STA
    netmask.fromString(configDev.staMS);            // маска подсети
    gateway.fromString(configDev.staGW);            // IP адрес шлюза
    WiFi.config(address, gateway, netmask);         // Указание сетевых параметров
  }
  int i = 15;                                       // Устанавливаем 15 попыток подключения
  while (i) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    } else {
      delay(1000);
      Serial.print(".");
      i--;
    }
  }
  return false;
}

// ********************************************************************** //
// ЗАПУСК РЕЖИМА AP+STA
void startAPSTA() {
  WiFi.mode(WIFI_AP_STA);                           // Установка режима AP+STA
  IPAddress address, netmask, gateway;              // Задание IP адресов
  address.fromString(configDev.apIP);               // IP адрес модуля в режиме AP
  netmask.fromString(configDev.apMS);               // Маска подсети
  gateway.fromString(configDev.apGW);               // IP адрес шлюза
  WiFi.softAPConfig(address, gateway, netmask);     // Указание сетевых параметров
  WiFi.softAP(configDev.apSSID, configDev.apPSK);   // Указание параметров авторизации
}

// ********************************************************************** //
// ПРОВЕРКА ПОДКЛЮЧЕНИЯ К СЕТИ
void checkConnect() {
  flagWConnect = false;
  if (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED)  {         // Запуск режима AP+STA при отсутствии соединения в режиме STA
    sendUART("SYS", "ConStatus", "Disconnected");
    return wifiInit();
  }
  if (WiFi.getMode() == WIFI_AP_STA && !WiFi.softAPgetStationNum()) {         // Подключение к появившейся сети, если нет подключенных клиентов
    int n = WiFi.scanComplete();
    if (n == -2) {
      WiFi.scanNetworks(true);
    } else if (n) {
      for (int i = 0; i < n; ++i) {
        if (WiFi.SSID(i) == configDev.staSSID) {
          WiFi.scanDelete();
          return wifiInit();
        }
      }
      WiFi.scanDelete();
      if (WiFi.scanComplete() == -2)
        WiFi.scanNetworks(true);
    }
  }
}

// ********************************************************************** //
// Сканирование точек доступа
void printListAPs(int networksFound) {
  DynamicJsonDocument doc(2048);
  JsonArray apList = doc.to<JsonArray>();
  for (int i = 0; i < networksFound; ++i) {
    JsonObject net = apList.createNestedObject();
    net["ssid"] = WiFi.SSID(i);
    net["rssi"] = WiFi.RSSI(i);
    net["ch"] = WiFi.channel(i);
  }
  String netList;
  serializeJson(apList, netList);
  sendUART("OK!", "netList", netList.c_str());
  sendWS(NULL, "NetList", netList);
}

// ********************************************************************** //
// ВЫВОД СТАТУСА (определиться со списком выводимых параметров)
void wifiPrintStatus() {
  sendUART("SYS", "ipDev", (WiFi.getMode() == WIFI_STA ? WiFi.localIP() : WiFi.softAPIP()).toString().c_str());
}

// ********************************************************************** //
// Установка флага проверки подключения к сети
void checkConnectFlag() {
  flagWConnect = true;
}
