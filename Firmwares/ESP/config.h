
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEVICE_ID           "35"              // ID устройства https://docs.google.com/document/d/1YQLu0gtEd-ghIbiy-TvrDSWPEQVXVrVSjW4NUSNMrjo/edit?usp=sharing
#define DEVICE_TYPE         "SWITCHING"         // Тип устройства
#define DEVICE_MODEL        "WiFi-GW"        // Модель устройства
#define VERSION             0                 // Версия прошивки
// ================================================================================ //
#define CONFIG_PATH         "/config.json"      // Путь к локальному файлу настроек

#define IP_DHCP             0                   // Динамические параметры сети
#define IP_STATIC           1                   // Статические параметры сети

#define STATUS_LED_PIN      12                  // Светодиод "Статус"
#define RESET_MCU_PIN       5                   // Ножка для сброса MCU IO5

struct Config {
  char di[3] = DEVICE_ID;                 // ID устройства
  char da[7];                             // Адрес устройства (применяется в Радио сети)
  char dn[17];                            // Имя устройства
  char dc[4];                             // Цветовая зона
  char dl[3];                             // Язык интерфейса
  char dls[32];                           // Поддерживаемые языки ["en","es","fr","it","pt","ru"], каждый язык +5 символов
  char dv[2];                             // Тема визуального оформления интерфейса
  char du[17];                            // Имя пользователя для доступа к закрытым областям
  char dp[17];                            // Пароль для доступа к закрытым областям
  char rch[3];                            // Частотный канал в Радио сети
  char staSSID[33];                       // Точка доступа
  char staPSK[65];                        // Пароль Точки доступа
  bool staIPMode;                         // Режим параметров сети, статические или динамисеские
  char staIP[16];                         // IP адрес при статических параметрах сети
  char staMS[16];                         // Маска подсети при статических параметрах сети
  char staGW[16];                         // IP адрес шлюза при статических параметрах сети
  char apSSID[17];                        // Идентификатор устройства в режиме Точка доступа
  char apPSK[17];                         // Пароль устройства в режиме Точка доступа
  char apIP[16];                          // IP адрес устройства в режиме Точка доступа
  char apMS[16];                          // Маска подсети устройства в режиме Точка доступа
  char apGW[16];                          // IP адрес шлюза в режиме Точка доступа
  // ... добавить пользовательские переменные (имена IO и т.п.)
  char in1[17];
  char in2[17];
};
extern struct Config configDev;

// ФУНКЦИИ
void loadConfig(int target);              // Загрузка параметров из файла config.json
void saveConfig();                        // Сохранение параметров в файл config.json


#endif
