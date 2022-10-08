
#include "searchDevs.h"

// ********************************************************************** //
// ИНИЦИАЛИЗАЦИЯ UDP, ОБРАБОТКА ОТВЕТОВ, ДОБАВЛЕНИЕ НОВЫХ УСТРОЙСТВ
void udpInit() {
  if (udp.listenMulticast(IPAddress(239, 255, 255, 250), 1900) && WiFi.status() == WL_CONNECTED) {
    udp.onPacket([](AsyncUDPPacket packet) {                    // Функция обратного вызова (прием пакета)
      if (flagHandlePacket) {                                   // Если разрешен прием пакетов - обрабатываем входящий пакет
        const char * inPack = (char*)packet.data();             // Забираем принятый пакет в inPack
        if (strstr(inPack, "SERVER: SOI Tech")) {               // Если в ответе есть SERVER: SOI Tech - обрабатываем пакет
          for (int i = 0; i < 100; i++) {                       // Перебираем массив IP адресов
            if (ipList[i] == NULL) {                            // Если дошли до конца списка IP адресов и не нашли совпадений, то
              ipList[i] = packet.remoteIP().toString();         // добавляем IP адрес ответившего устройства в массив,
              char * temp = strstr(inPack, "\r\n\r\n,{") + 4;   // находим указатель на начало данных в ответе,
              char * temp2 = strstr(temp, "\r\n\r\n");          // определяем конец строки,
              temp2[0] = 0;                                     // затираем все после конца строки,
              devsListTemp += temp;                             // добавляем объект (полезные данные) к списку устройств
              return;
            }
            if (ipList[i] == packet.remoteIP().toString())      // Если IP ответившего устройства уже есть в списке, то
              return;                                           // игнорируем пакет
          }
        }
      }
      packet.printf(                                            // Ответ на широковещательный запрос
        "HTTP/1.1 200 OK\r\n"                                   // Формируем шаблон ответа
        "CACHE-CONTROL: max-age=1800\r\n"                       // и добавляем данные об устройстве
        "EXT: \r\n"                                             // после двойных переносов строк (\r\n\r\n)
        "LOCATION: http://%s:80/description.xml\r\n"            // для того, чтоб не делать
        "SERVER: %s/1.0 UPnP/1.1 %s\r\n"                        // дополнительный запрос на получение
        "ST: upnp:rootdevice\r\n"                               // данных об устройстве
        "USN: uuid:EDC3B25E-C2D0-411E-B04B-F9BD5E%02X::upnp:rootdevice\r\n\r\n"
        ",{\"ip\":\"%s\",\"da\":\"%s\",\"dt\":\"%s\",\"dm\":\"%s\",\"dn\":\"%s\",\"dc\":\"%s\"}\r\n\r\n",  // JSON Объект с данными об устройстве
        WiFi.localIP().toString().c_str(), "SOI Tech", DEVICE_MODEL, ESP.getChipId(), WiFi.localIP().toString().c_str(), configDev.da, DEVICE_TYPE, DEVICE_MODEL, configDev.dn, configDev.dc
      );
    });
  } else {
    sendUART("SYS", "UDP", "nOK");
  }
}

// ********************************************************************** //
// СКАНИРОВАНИЕ УСТРОЙСТВ В СЕТИ
void findDevices() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(STATUS_LED_PIN, HIGH);                           // Зажигаем светодиод СТАТУС
    // Добавляем данные о локальном устройстве в список устройств
    devsListTemp = "[{\"ip\":\""; devsListTemp += WiFi.localIP().toString(); devsListTemp += "\",";
    devsListTemp += "\"da\":\""; devsListTemp += configDev.da; devsListTemp += "\",";
    devsListTemp += "\"dt\":\""; devsListTemp += DEVICE_TYPE; devsListTemp += "\",";
    devsListTemp += "\"dm\":\""; devsListTemp += DEVICE_MODEL; devsListTemp += "\",";
    devsListTemp += "\"dn\":\""; devsListTemp += configDev.dn; devsListTemp += "\",";
    devsListTemp += "\"dc\":\""; devsListTemp += configDev.dc; devsListTemp += "\"}";
    ipList[0] = WiFi.localIP().toString();                        // а так же IP адрес локального устройства в список IP адресов
    udp.broadcastTo(                                              // Формируем широковещательный запрос
      "M-SEARCH * HTTP/1.1\r\n"                                   // Фрейма M-SEARCH
      "Host:239.255.255.250:1900\r\n"                             // IP-адрес и порт для мультивещания
      "ST:upnp:rootdevice\r\n"                                    // Поиск корневых устройств
      "Man:\"ssdp:discover\"\r\n"
      "MX:3\r\n\r\n", 1900
    );
    flagHandlePacket = true;                                      // Разрешаем обработку принятых пакетов
    waitScan.attach(4, udpScanStop);                              // Отключаем сканер через 4 секунды
  }
}

// ********************************************************************** //
// ОТКЛЮЧЕНИЕ СКАНЕРА
void udpScanStop() {
  flagHandlePacket = false;                                     // Запрещаем обработку принятых пакетов
  waitScan.detach();                                            // Выключаем диспетчер задержки
  digitalWrite(STATUS_LED_PIN, LOW);                            // Гасим светодиод СТАТУС
  devsListTemp += "]";                                          // Закрываем массив списка устройств
  devsList = devsListTemp;                                      // Формируем итоговый список устройств.
  devsListTemp = "";                                            // Чистим временный буфер списка устройств
  for (int i = 0; i < 100; i++) {                               // Чистим список IP адресов устройств
    ipList[i] = "";
  }
  sendWS(NULL, "DevsList", devsList);                           // Возвращаем список найденных устройств
}
