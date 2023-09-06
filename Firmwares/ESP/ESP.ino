// ================================================================================ //
// SOI-WIFI - БАЗОВЫЙ WIFI МОДУЛЬ                                                   //
// Ядро https://github.com/esp8266/Arduino                              3.1.2       //
// Загрузчик FS https://github.com/esp8266/arduino-esp8266fs-plugin                 //
// JSON https://github.com/bblanchon/ArduinoJson                        6.21.3      //
// ESPAsyncWebServer https://github.com/me-no-dev/ESPAsyncWebServer                 //
// ESPAsyncTCP https://github.com/me-no-dev/ESPAsyncTCP                             //
// ESPAsyncUDP https://github.com/me-no-dev/ESPAsyncUDP                             //
// Возможна коррекция файла boards.txt                                              //
// LIGHTING    00-2F                                                                //
// SWITCHING   30-4F                                                                //
// CLIMATE     50-6F                                                                //
// SECURITY    70-9F                                                                //
// MEDIA       A0-BF                                                                //
// SENSORS     C0-DF                                                                //
// API https://docs.google.com/document/d/1qBwquRtjmVb4Zq6yYob40aukkdVURFmAt3-TLdhq6yo/edit //
// ================================================================================ //


// ********************************************************************** //
#include <ESPAsyncWebServer.h>  // НЕ ПЕРЕНОСИТЬ, не компилируется!
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamString.h>
#include <FS.h>
#include <Ticker.h>
// ================================================================================ //
#include "config.h"
#include "uart.h"
#include "wifi.h"
#include "server.h"
#include "searchDevs.h"
#include "sysFun.h"
#include "update.h"
// ********************************************************************** //

#define DEVICE_URL          "http://m-art.by/production/" DEVICE_ID "/"   // Путь к изделию на сайте производителя
char OTA_PATH[16];                              // Путь к файлу id.txt на сервере
char OTA_PATH_FLASH[19];                        // Переменная для хранения пути к прошивке Flash MCU
char OTA_PATH_EEPROM[19];                       // Переменная для хранения пути к прошивке EEPROM MCU

bool flagCheckUpdate = false;                   // Проверка наличия обновления на сервере
bool flagWConnect = false;                      // Проверка состояния подключения

// ДИСПЕТЧЕР ЗАДАЧ
Ticker wifiTicker;                              // Проверка подключения к сети
Ticker udpTicker;                               // Сканирования устройств в сети
Ticker waitScan;                                // Задержки при сканировании сети (между широковещательным запросом и завершением прослушивания)
Ticker updateTicker;                            // Проверки наличия обновления на сервере
Ticker waitMcuUpdateProcess;                    // Ожидание готовности загрузчика MCU к прошивке
Ticker ledTicker;                               // Моргание светодиодом STATUS
WiFiClient wifiClient;

// ********************************************************************** //
void setup() {
  pinMode(STATUS_LED_PIN, OUTPUT);              // Пин светодиода СТАТУС на Выход
  pinMode(RESET_MCU_PIN, OUTPUT);               // Пин сброса MCU на Выход
  digitalWrite(RESET_MCU_PIN, HIGH);            // Устанавливаем 1
  uartInit(115200);                             // Инициализация UART
  if (SPIFFS.begin()) {                         // Если Файловая система инициализирована
    loadConfig(0);                              // Загрузка параметров
    WiFi.persistent(false);                     // Запись во Flash только при изменении параметров подключения
    sprintf(OTA_PATH, "/Updates/%s.txt", DEVICE_ID);            // Обновляем путь к файлу id.txt на сервере
    sprintf(OTA_PATH_FLASH, "/Updates/%s-MF.bin", DEVICE_ID);   // Путь к файлу BIN на сервере (Flash)
    sprintf(OTA_PATH_EEPROM, "/Updates/%s-ME.bin", DEVICE_ID);  // Путь к файлу BIN на сервере (EEPROM)
    wifiInit();                                 // Инициализация WiFi
    serverInit();                               // Инициализация Web сервера
    wifiTicker.attach(30, checkConnectFlag);    // Проверка статуса подключения каждые 30 сек.
    udpTicker.attach(300, findDevices);         // Поиск устройств в сети каждые 5 минут
    updateTicker.attach(3600, checkUpdateFlag); // Прверка обновлений на сервере каждые 1 часа
    sendUART("GET", "API", NULL);
    delay(100);
    sendUART("GET", "SC", NULL);
    delay(100);
    sendUART("SYS", "FFS", "OK");
  } else {
    sendUART("SYS", "FFS", "nOK");
    ESP.restart();
  }
}

// ********************************************************************** //
void loop() {
  uartWork();                                 // Работа UART

  if (flagWConnect)                           // Проверяем флаг проверки состояния подключения, если установлен, то
    checkConnect();                           // проверяем состояние подключения к сети

//  if (flagCheckUpdate)                        // Проверяем флаг проверки наличия обновления на сервере, если установлен, то
//    getServerData(OTA_PATH);                  // запускаем скачивание файла id.txt с сервера (в нем версия и md5 файлов прошивок)
//  if (flagGetMCUF)                            // Проверяем флаг получения прошивки FLASH для MCU, если установлен, то
//    getServerData(OTA_PATH_FLASH);            // запускаем скачивание файла с сервера
//  if (flagGetMCUE)                            // Проверяем флаг получения прошивки EEPROM для MCU, если установлен, то
//    getServerData(OTA_PATH_EEPROM);           // запускаем скачивание файла с сервера

  if (flagCheckBINs) {                        // Проверка md5 только для скачанных файлов с сервера
    flagCheckBINs = false;                    // Сбрасываем флаг проверки контрольных сумм BIN файлов MCU
    if (checkBin(MCU_FLASH) && checkBin(MCU_EEPROM)) {  // Проверяем контрольные суммы, если соответствуют, то
      sendUART("OK!", "md5-MCU", NULL);       // сообщение об удачной проверке
      if (flagUpdate)                         // Проверяем, запущено ли обновление, если запущено, то
        flagUpdateMCU = true;                 // запускаем обновление MCU
    } else {                                  // В противном случае
      SPIFFS.remove(MCU_FLASH);               // удаляем несоответствующий файл прошивки Flash
      SPIFFS.remove(MCU_EEPROM);              // удаляем несоответствующий файл прошивки EEPROM
      sendUART("ER!", "md5-MCU", NULL);       // и сообщаем об ошибке
    }
  }

  if (flagUpdateMCU)                          // Проверяем флаг обновления MCU, если установлен, то
    updateMCU();                              // запускаем обновление MCU
  if (flagUpdateESP)                          // Проверяем флаг обновления ESP, если установлен, то
    updateESP();                              // запускаем обновление ESP
}

// ********************************************************************** //
// Установка флага запуска проверки наличия обновления на сервере
void checkUpdateFlag() {
  flagCheckUpdate = true;
}

// ********************************************************************** //
// Смена состояния светодиода STATUS
void changeLedState() {
  digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
}
