
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
const char index_htm[] PROGMEM = "<!DOCTYPE html>"
"<html>"
  "<head>"
    "<title>SOI Tech | Device Manager</title>"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<link rel=\"shortcut icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEACABoBQAAFgAAACgAAAAQAAAAIAAAAAEACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/plUAP2aWAD4pW0A9at5ACHchwAi3YgAJt2JADDcjgA/3JUAT9ycAFPdngBW3Z8Aat2oAHHdqwBz3awA8LSMAOu+oQCJ3LYAjt25AOrApADlyrgApNzDAKjdxQC93M4Ay9zUAM/d1gDR3NcA1tzZANnc2wDc3NwA4tLGAOPe2wDj3twA1ODbANnh3QDi4uIA4eTjAOXk4wDl5eUA6urqAO/v7wDx8fEA9fX1APr6+gD9/f0A0fv/AP///wAAAAAAAC8hAABQNwAAcEwAAJBjAACweQAAz48AAPCmABH/tAAx/74AUf/IAHH/0wCR/9wAsf/lANH/8AD///8AAAAAAAAvDgAAUBgAAHAiAACQLAAAsDYAAM9AAADwSgAR/1sAMf9xAFH/hwBx/50Akf+yALH/yQDR/98A////AAAAAAACLwAABFAAAAZwAAAIkAAACrAAAAvPAAAO8AAAIP8SAD3/MQBb/1EAef9xAJj/kQC1/7EA1P/RAP///wAAAAAAFC8AACJQAAAwcAAAPZAAAEywAABZzwAAZ/AAAHj/EQCK/zEAnP9RAK7/cQDA/5EA0v+xAOT/0QD///8AAAAAACYvAABAUAAAWnAAAHSQAACOsAAAqc8AAMLwAADR/xEA2P8xAN7/UQDj/3EA6f+RAO//sQD2/9EA////AAAAAAAvJgAAUEEAAHBbAACQdAAAsI4AAM+pAADwwwAA/9IRAP/YMQD/3VEA/+RxAP/qkQD/8LEA//bRAP///wAAAAAALxQAAFAiAABwMAAAkD4AALBNAADPWwAA8GkAAP95EQD/ijEA/51RAP+vcQD/wZEA/9KxAP/l0QD///8AAAAAAC8DAABQBAAAcAYAAJAJAACwCgAAzwwAAPAOAAD/IBIA/z4xAP9cUQD/enEA/5eRAP+2sQD/1NEA////AAAAAAAvAA4AUAAXAHAAIQCQACsAsAA2AM8AQADwAEkA/xFaAP8xcAD/UYYA/3GcAP+RsgD/scgA/9HfAP///wAAAAAALwAgAFAANgBwAEwAkABiALAAeADPAI4A8ACkAP8RswD/Mb4A/1HHAP9x0QD/kdwA/7HlAP/R8AD///8AAAAAACwALwBLAFAAaQBwAIcAkAClALAAxADPAOEA8ADwEf8A8jH/APRR/wD2cf8A95H/APmx/wD70f8A////AAAAAAAbAC8ALQBQAD8AcABSAJAAYwCwAHYAzwCIAPAAmRH/AKYx/wC0Uf8AwnH/AM+R/wDcsf8A69H/AP///wAAAAAACAAvAA4AUAAVAHAAGwCQACEAsAAmAM8ALADwAD4R/wBYMf8AcVH/AIxx/wCmkf8Av7H/ANrR/wD///8AAAAAAAAoIR8fISgAAAAAAAAAACgVBAICAQIEFSgAAAAAACcQAgECAQEBAQEQJwAAACgUAwMDAwMDAwMDAxEoAAAeIR4eHh4eHiQeHh4eHgAoKR4eHh4eHh4eHh4kHQ8oJS0oHh4eHh4eHh4eHhMHIyQtLR4eHh4eHh4eHhoIBhgkLS8pHh4eHh4eHh4OBgYYJC0vLSQeHh4eHh4YBwYHIygrLy8rHh4eHh4eDAYGCygAJy0vLSceHh4eFgYGBhcAACgqLy8sHh4eHAkGBg0oAAAAJyotLygeHhIFBg0lAAAAAAAoJystIRkIChYoAAAAAAAAAAAoJB4bIigAAAAAAPgfAADgBwAAwAMAAIABAACAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAEAAIABAADAAwAA4AcAAPgfAAA=\" />"
    "<link type=\"text/css\" rel=\"stylesheet\" href=\"style.css\">"
  "</head>"
  "<body class=\"loading\">"
    "<div id=\"app\"></div>"
    "<div id=\"loader\"></div>"
    "<script src=\"main.js\"></script>"
  "</body>"
"</html>";

//#define style_css_gz_len 6233
//static const uint8_t style_css_gz[] PROGMEM = {
// HEX Data 0x64, 0x99...
//};

//#define main_js_gz_len 23088
//static const uint8_t main_js_gz[] PROGMEM = {
// HEX Data 0x64, 0x99...
//};

// ГЕНЕРАТОР ИКОНКИ УСТРОЙСТВА
const char icon[] PROGMEM = "<img style=\"height:4em;\" src='data:image/svg+xml;utf8,"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" height=\"100\" width=\"100\">"
     "<defs>"
        "<marker id=\"a\" orient=\"auto\" overflow=\"visible\"><path class=\"c e i\" d=\"M.508 0l-3.46 2v-4z\"/></marker>"
        "<marker id=\"b\" orient=\"auto\" overflow=\"visible\"><path class=\"c e i\" d=\"M.508 0l-3.46 2v-4z\"/></marker>"
     "</defs>"
     "<style>"
            "circle,path,rect{stroke-linecap:round;stroke-linejoin:round}"
            ".a{fill:none}.b{fill:#e66}.c{fill:#777}"
            ".d{stroke:#e66}.e{stroke:#777}"
            ".f{paint-order:stroke fill markers}"
            ".g{cx:50;cy:50}"
            ".h{stroke-width:2}"
            ".i{stroke-width:.5}"
     "</style>"
     "<circle class=\"a d f g\" r=\"46.5\" stroke-width=\"7\"/>"
     "<circle class=\"a f g\" r=\"49.5\" stroke=\"#555\" stroke-width=\"1\"/>"
     "<path class=\"a d\" d=\"M42.5 44.535v20.93M62.464 44.5h-9.928M52.5 44.535v20.93M62.464 54.5h-9.928\" stroke-width=\"5\"/>"
     "<rect class=\"b e f\" width=\"11\" height=\"11\" x=\"66\" y=\"67\" rx=\"3\" ry=\"3\" stroke-width=\"4\"/>"
     "<rect class=\"a e f h\" width=\"13\" height=\"13\" x=\"21\" y=\"21\" rx=\"7\" ry=\"7\"/>"
     "<path class=\"a e f h\" d=\"M37.963 18.99a33.264 33.264 0 0 1 35.482 7.414 33.264 33.264 0 0 1 7.64 35.434\" marker-end=\"url(#a)\"/>"
     "<path class=\"a e f h\" d=\"M-61.835-82.277a33.264 33.264 0 0 1 35.274 7.67 33.264 33.264 0 0 1 7.512 35.306\" transform=\"scale(-1)\" marker-end=\"url(#b)\"/>"
     "<circle class=\"b f\" cx=\"42.5\" cy=\"37.5\" r=\"2.5\"/>"
  "</svg>'>";

// ГЕНЕРАТОР ВСТАВКИ В БЛОК НАЙДЕННОГО УСТРОЙСТВА
static const char dev[] PROGMEM =
  "<p class=\"bold\">Режим работы</p>"
  "<button id=\"gmd\" class=\"button w30 red large\" value=\"0\">Выключено</button>"
  "<button id=\"gmd\" class=\"button w30 red large\" value=\"1\">Включено</button>"
  "<button id=\"gmd\" class=\"button w30 red large\" value=\"2\">Имитация</button>";

// ГЕНЕРАТОР ФАЙЛА description.xml
const char* ssdpTemplate =
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u/</URLBase>"
    "<device>"
      "<deviceType>upnp:rootdevice</deviceType>"
      "<friendlyName>%s | %s</friendlyName>"
      "<manufacturer>%s</manufacturer>"
      "<manufacturerURL>%s</manufacturerURL>"
      "<modelDescription>UPnP Lighting Control</modelDescription>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>%s</modelURL>"
      "<serialNumber>%s</serialNumber>"
      "<UDN>uuid:edc3b25e-c2d0-411e-b04b-f9bd5e%02x%02x%02x</UDN>"
      "<presentationURL>/</presentationURL>"
    "</device>"
    "<iconList>"
      "<icon>"
        "<mimetype>image/png</mimetype>"
        "<height>48</height>"
        "<width>48</width>"
        "<depth>24</depth>"
        "<url>icon48.png</url>"
      "</icon>"
      "<icon>"
        "<mimetype>image/png</mimetype>"
        "<height>120</height>"
        "<width>120</width>"
        "<depth>24</depth>"
        "<url>icon120.png</url>"
      "</icon>"
    "</iconList>"
  "</root>\r\n"
  "\r\n";


#endif
