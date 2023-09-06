
// Структура файла с версией и контрольными суммами
// 1:68e13dfc0751eff44b04294ecf884d43-3fd4bb7b73c110ac3e03c2c9fe0fbed8-d5dd76bb948b2a5feec0e3c25b4eaa77-1eebc93e0a94374bc22bb807bc5dd04f
// latVer : md5FlashLatest - md5EEPROMLatest - md5ffsLatest - md5espLatest

#ifndef _SERVER_H_
#define _SERVER_H_

#include <ESPAsyncWebServer.h>

// СЕРВЕР
AsyncWebServer server(80);                    // Инициализируем http сервер на порту 80
AsyncWebSocket ws("/ws");                     // Инициализируем WebSocket Server (ws://ip/ws)
AsyncWebSocketClient * client;                // Инициализируем WebSocket Client
static AsyncClient * aClient =  NULL;         // Инициализируем асинхронный клиент
int latVer =                    VERSION;      // Начальное значение последней версии прошивки
char md5FlashLatest[33] =       "";           // MD5 последней версии прошивки MCU (Flash)
char md5EEPROMLatest[33] =      "";           // MD5 последней версии прошивки MCU (EEPROM)
char md5ffsLatest[33] =         "";           // MD5 последней версии прошивки FS
char md5espLatest[33] =         "";           // MD5 последней версии прошивки ESP

// ФЛАГИ
bool gotHeaders =               false;        // Флаг удаления заголовков из ответов от сервера SOI Tech
bool flagGetMCUF =              false;        // Запуск получения BIN файла прошивки MCU Flash с сервера
bool flagGetMCUE =              false;        // Запуск получения BIN файла прошивки MCU EEPROM с сервера
bool flagCheckBINs =            false;        // Запуск проверки md5 BIN файлов MCU

// ФУНКЦИИ
void serverInit();                            // Инициализация сервера
void getServerData(const char *path);

// ОБРАБОТЧИК СОБЫТИЙ WEBSOCKET
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ПЕРЕДАЧА ДАННЫХ В WEBSOCKET
void sendWS(AsyncWebSocketClient *client, const char *argument, String data); // Передаем объект: { argument: data }

// ВАЛИДАЦИЯ И ТРАНСФОРМАЦИЯ IP АДРЕСА
String handleIP (String ip);  // Приводит адрес к формату 0-255.0-255.0-255.0-255, если исходные данные не верные - возвращает пустую строку

// ГЕНЕРАТОР СТРАНИЦЫ index.html
const char index_htm[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <title>Peleng | Device Manager</title>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="shortcut icon" href="data:image/x-icon;base64,AAABAAEAEBAAAAEACABoBQAAFgAAACgAAAAQAAAAIAAAAAEACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/plUAP2aWAD4pW0A9at5ACHchwAi3YgAJt2JADDcjgA/3JUAT9ycAFPdngBW3Z8Aat2oAHHdqwBz3awA8LSMAOu+oQCJ3LYAjt25AOrApADlyrgApNzDAKjdxQC93M4Ay9zUAM/d1gDR3NcA1tzZANnc2wDc3NwA4tLGAOPe2wDj3twA1ODbANnh3QDi4uIA4eTjAOXk4wDl5eUA6urqAO/v7wDx8fEA9fX1APr6+gD9/f0A0fv/AP///wAAAAAAAC8hAABQNwAAcEwAAJBjAACweQAAz48AAPCmABH/tAAx/74AUf/IAHH/0wCR/9wAsf/lANH/8AD///8AAAAAAAAvDgAAUBgAAHAiAACQLAAAsDYAAM9AAADwSgAR/1sAMf9xAFH/hwBx/50Akf+yALH/yQDR/98A////AAAAAAACLwAABFAAAAZwAAAIkAAACrAAAAvPAAAO8AAAIP8SAD3/MQBb/1EAef9xAJj/kQC1/7EA1P/RAP///wAAAAAAFC8AACJQAAAwcAAAPZAAAEywAABZzwAAZ/AAAHj/EQCK/zEAnP9RAK7/cQDA/5EA0v+xAOT/0QD///8AAAAAACYvAABAUAAAWnAAAHSQAACOsAAAqc8AAMLwAADR/xEA2P8xAN7/UQDj/3EA6f+RAO//sQD2/9EA////AAAAAAAvJgAAUEEAAHBbAACQdAAAsI4AAM+pAADwwwAA/9IRAP/YMQD/3VEA/+RxAP/qkQD/8LEA//bRAP///wAAAAAALxQAAFAiAABwMAAAkD4AALBNAADPWwAA8GkAAP95EQD/ijEA/51RAP+vcQD/wZEA/9KxAP/l0QD///8AAAAAAC8DAABQBAAAcAYAAJAJAACwCgAAzwwAAPAOAAD/IBIA/z4xAP9cUQD/enEA/5eRAP+2sQD/1NEA////AAAAAAAvAA4AUAAXAHAAIQCQACsAsAA2AM8AQADwAEkA/xFaAP8xcAD/UYYA/3GcAP+RsgD/scgA/9HfAP///wAAAAAALwAgAFAANgBwAEwAkABiALAAeADPAI4A8ACkAP8RswD/Mb4A/1HHAP9x0QD/kdwA/7HlAP/R8AD///8AAAAAACwALwBLAFAAaQBwAIcAkAClALAAxADPAOEA8ADwEf8A8jH/APRR/wD2cf8A95H/APmx/wD70f8A////AAAAAAAbAC8ALQBQAD8AcABSAJAAYwCwAHYAzwCIAPAAmRH/AKYx/wC0Uf8AwnH/AM+R/wDcsf8A69H/AP///wAAAAAACAAvAA4AUAAVAHAAGwCQACEAsAAmAM8ALADwAD4R/wBYMf8AcVH/AIxx/wCmkf8Av7H/ANrR/wD///8AAAAAAAAoIR8fISgAAAAAAAAAACgVBAICAQIEFSgAAAAAACcQAgECAQEBAQEQJwAAACgUAwMDAwMDAwMDAxEoAAAeIR4eHh4eHiQeHh4eHgAoKR4eHh4eHh4eHh4kHQ8oJS0oHh4eHh4eHh4eHhMHIyQtLR4eHh4eHh4eHhoIBhgkLS8pHh4eHh4eHh4OBgYYJC0vLSQeHh4eHh4YBwYHIygrLy8rHh4eHh4eDAYGCygAJy0vLSceHh4eFgYGBhcAACgqLy8sHh4eHAkGBg0oAAAAJyotLygeHhIFBg0lAAAAAAAoJystIRkIChYoAAAAAAAAAAAoJB4bIigAAAAAAPgfAADgBwAAwAMAAIABAACAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAEAAIABAADAAwAA4AcAAPgfAAA=" />
    <link type="text/css" rel="stylesheet" href="style.css">
  </head>
  <body class="loading">
    <div id="app"></div>
    <div id="loader"></div>
    <script src="main.js"></script>
  </body>
</html>
)=====";

//#define style_css_gz_len 6233
//static const uint8_t style_css_gz[] PROGMEM = {
// HEX Data 0x64, 0x99...
//};

//#define main_js_gz_len 23088
//static const uint8_t main_js_gz[] PROGMEM = {
// HEX Data 0x64, 0x99...
//};

// ГЕНЕРАТОР ИКОНКИ УСТРОЙСТВА
static const char icon[] PROGMEM = R"=====(
  <img alt="" style="height:4em;" src="data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxMDAgMTAwIiBoZWlnaHQ9IjEwMCIgd2lkdGg9IjEwMCI+DQogIDxkZWZzPg0KICAgIDxtYXJrZXIgaWQ9ImEiIG9yaWVudD0iYXV0byIgb3ZlcmZsb3c9InZpc2libGUiPjxwYXRoIGNsYXNzPSJjIGUgaSIgZD0iTS41MDggMGwtMy40NiAydi00eiIvPjwvbWFya2VyPg0KICAgIDxtYXJrZXIgaWQ9ImIiIG9yaWVudD0iYXV0byIgb3ZlcmZsb3c9InZpc2libGUiPjxwYXRoIGNsYXNzPSJjIGUgaSIgZD0iTS41MDggMGwtMy40NiAydi00eiIvPjwvbWFya2VyPg0KICA8L2RlZnM+DQogIDxzdHlsZT4NCiAgICBjaXJjbGUscGF0aCxyZWN0e3N0cm9rZS1saW5lY2FwOnJvdW5kO3N0cm9rZS1saW5lam9pbjpyb3VuZH0NCiAgICAuYXtmaWxsOm5vbmV9LmJ7ZmlsbDojZTY2fS5je2ZpbGw6Izc3N30NCiAgICAuZHtzdHJva2U6I2U2Nn0uZXtzdHJva2U6Izc3N30NCiAgICAuZntwYWludC1vcmRlcjpzdHJva2UgZmlsbCBtYXJrZXJzfQ0KICAgIC5ne2N4OjUwO2N5OjUwfQ0KICAgIC5oe3N0cm9rZS13aWR0aDoyfQ0KICAgIC5pe3N0cm9rZS13aWR0aDouNX0NCiAgPC9zdHlsZT4NCiAgPGNpcmNsZSBjbGFzcz0iYSBkIGYgZyIgcj0iNDYuNSIgc3Ryb2tlLXdpZHRoPSI3Ii8+DQogIDxjaXJjbGUgY2xhc3M9ImEgZiBnIiByPSI0OS41IiBzdHJva2U9IiM1NTUiIHN0cm9rZS13aWR0aD0iMSIvPg0KICA8cGF0aCBjbGFzcz0iYSBkIiBkPSJNNDIuNSA0NC41MzV2MjAuOTNNNjIuNDY0IDQ0LjVoLTkuOTI4TTUyLjUgNDQuNTM1djIwLjkzTTYyLjQ2NCA1NC41aC05LjkyOCIgc3Ryb2tlLXdpZHRoPSI1Ii8+DQogIDxyZWN0IGNsYXNzPSJiIGUgZiIgd2lkdGg9IjExIiBoZWlnaHQ9IjExIiB4PSI2NiIgeT0iNjciIHJ4PSIzIiByeT0iMyIgc3Ryb2tlLXdpZHRoPSI0Ii8+DQogIDxyZWN0IGNsYXNzPSJhIGUgZiBoIiB3aWR0aD0iMTMiIGhlaWdodD0iMTMiIHg9IjIxIiB5PSIyMSIgcng9IjciIHJ5PSI3Ii8+DQogIDxwYXRoIGNsYXNzPSJhIGUgZiBoIiBkPSJNMzcuOTYzIDE4Ljk5YTMzLjI2NCAzMy4yNjQgMCAwIDEgMzUuNDgyIDcuNDE0IDMzLjI2NCAzMy4yNjQgMCAwIDEgNy42NCAzNS40MzQiIG1hcmtlci1lbmQ9InVybCgjYSkiLz4NCiAgPHBhdGggY2xhc3M9ImEgZSBmIGgiIGQ9Ik0tNjEuODM1LTgyLjI3N2EzMy4yNjQgMzMuMjY0IDAgMCAxIDM1LjI3NCA3LjY3IDMzLjI2NCAzMy4yNjQgMCAwIDEgNy41MTIgMzUuMzA2IiB0cmFuc2Zvcm09InNjYWxlKC0xKSIgbWFya2VyLWVuZD0idXJsKCNiKSIvPg0KICA8Y2lyY2xlIGNsYXNzPSJiIGYiIGN4PSI0Mi41IiBjeT0iMzcuNSIgcj0iMi41Ii8+DQo8L3N2Zz4=" />
)=====";
//static const char icon[] PROGMEM = R"=====(
//<img style="height:4em;" src='data:image/svg+xml;utf8,
//<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100" height="100" width="100">
//  <defs>
//    <marker id="a" orient="auto" overflow="visible"><path class="c e i" d="M.508 0l-3.46 2v-4z"/></marker>
//    <marker id="b" orient="auto" overflow="visible"><path class="c e i" d="M.508 0l-3.46 2v-4z"/></marker>
//  </defs>
//  <style>
//    circle,path,rect{stroke-linecap:round;stroke-linejoin:round}
//    .a{fill:none}.b{fill:#e66}.c{fill:#777}
//    .d{stroke:#e66}.e{stroke:#777}
//    .f{paint-order:stroke fill markers}
//    .g{cx:50;cy:50}
//    .h{stroke-width:2}
//    .i{stroke-width:.5}
//  </style>
//  <circle class="a d f g" r="46.5" stroke-width="7"/>
//  <circle class="a f g" r="49.5" stroke="#555" stroke-width="1"/>
//  <path class="a d" d="M42.5 44.535v20.93M62.464 44.5h-9.928M52.5 44.535v20.93M62.464 54.5h-9.928" stroke-width="5"/>
//  <rect class="b e f" width="11" height="11" x="66" y="67" rx="3" ry="3" stroke-width="4"/>
//  <rect class="a e f h" width="13" height="13" x="21" y="21" rx="7" ry="7"/>
//  <path class="a e f h" d="M37.963 18.99a33.264 33.264 0 0 1 35.482 7.414 33.264 33.264 0 0 1 7.64 35.434" marker-end="url(#a)"/>
//  <path class="a e f h" d="M-61.835-82.277a33.264 33.264 0 0 1 35.274 7.67 33.264 33.264 0 0 1 7.512 35.306" transform="scale(-1)" marker-end="url(#b)"/>
//  <circle class="b f" cx="42.5" cy="37.5" r="2.5"/>
//</svg>'>
//)=====";

//static const char icon[] PROGMEM = "<img style=\"height:4em;\" src='data:image/svg+xml;utf8,%3Csvg xmlns=%5C\"http://www.w3.org/2000/svg%5C\" width=%5C\"50%5C\" height=%5C\"50%5C\" viewBox=%5C\"0 0 13.229166 13.229166%5C\" id=%5C\"icon%5C\"%3E%3Cstyle%3Epath,circle%7Bstroke-linecap:round;stroke-linejoin:round%7D.a%7Bstroke:%23d1cc19%7D.b%7Bcx:6.615;cy:6.615%7D.c%7Bfill:none%7D.d%7Bstroke:%23999;stroke-width:.3%7D%3C/style%3E%3Ccircle class=%5C\"a b%5C\" r=%5C\"5.821%5C\" fill=%5C\"%23fdfcdc%5C\" stroke-width=%5C\"1.5%5C\" paint-order=%5C\"stroke fill markers%5C\"/%3E%3Ccircle class=%5C\"b c%5C\" r=%5C\"6.548%5C\" stroke=%5C\"%23555%5C\" stroke-width=%5C\".1%5C\" paint-order=%5C\"stroke fill markers%5C\"/%3E%3Cpath class=%5C\"a c%5C\" d=%5C\"M2.738 9.914h7.606M4.198 8.604l6.85-.01M5.146 7.273h5.78M5.12 5.953h4.545M4.218 4.633h3.598M2.737 3.312l3.172.02%5C\" stroke-width=%5C\".6%5C\"/%3E%3Cpath class=%5C\"c d%5C\" d=%5C\"M9.454 7.962c3.273-2.95-.053-4.756-3.198-6.54M7.273 4.17V2.084M9.275 5.488V3.4M10.108 9.443V7.355%5C\"/%3E%3C/svg%3E'>";

// ГЕНЕРАТОР ВСТАВКИ В БЛОК НАЙДЕННОГО УСТРОЙСТВА
static const char dev[] PROGMEM = R"=====(
  <p class="bold">Режим работы</p>
  <button id="gmd" class="button w30 red large" value="0">Выключено</button>
  <button id="gmd" class="button w30 red large" value="1">Включено</button>
  <button id="gmd" class="button w30 red large" value="2">Имитация</button>
)=====";

// ГЕНЕРАТОР ФАЙЛА description.xml
static const char ssdpTemplate[] PROGMEM = R"=====(
  <?xml version="1.0"?>
  <root xmlns="urn:schemas-upnp-org:device-1-0">
    <specVersion>
      <major>1</major>
      <minor>0</minor>
    </specVersion>
    <URLBase>http://%u.%u.%u.%u/</URLBase>
    <device>
      <deviceType>upnp:rootdevice</deviceType>
      <friendlyName>%s | %s</friendlyName>
      <manufacturer>%s</manufacturer>
      <manufacturerURL>%s</manufacturerURL>
      <modelDescription>UPnP Lighting Control</modelDescription>
      <modelName>%s</modelName>
      <modelNumber>%s</modelNumber>
      <modelURL>%s</modelURL>
      <serialNumber>%s</serialNumber>
      <UDN>uuid:edc3b25e-c2d0-411e-b04b-f9bd5e%02x%02x%02x</UDN>
      <presentationURL>/</presentationURL>
    </device>
    <iconList>
      <icon>
        <mimetype>image/png</mimetype>
        <height>48</height>
        <width>48</width>
        <depth>24</depth>
        <url>icon48.png</url>
      </icon>
      <icon>
        <mimetype>image/png</mimetype>
        <height>120</height>
        <width>120</width>
        <depth>24</depth>
        <url>icon120.png</url>
      </icon>
    </iconList>
  </root>\r\n
  \r\n
)=====";


#endif
