
#include "update.h"

// ********************************************************************** //
// ОБНОВЛЕНИЕ MCU
void updateMCU() {
  flagUpdateMCU = false;                                    // Сброс флага запуска обновления прошивок MCU
  flagUpdate = true;                                        // Взводим флаг выполнения какого-либо обновления
  ledTicker.detach();                                       // Отключаем моргание светодиодом
  digitalWrite(STATUS_LED_PIN, HIGH);                       // Зажигаем светодиод
  if (checkBin(MCU_FLASH) && checkBin(MCU_EEPROM)) {        // Проверяем контрольные суммы файлов BIN MCU (только при полном обновлении)
    updateMCUInit();                                        // Запускаем процесс прошивки MCU
  } else {                                                  // Если при полном обновлении md5 BIN фалов не соответствует, то
    flagUpdateMCU = false;                                  // сбрасываем флаг обновления MCU
    flagUpdateESP = false;                                  // сбрасываем флаг обновления ESP
    flagUpdateALL = false;                                  // сбрасываем флаг обновления всего устройства
    updateMCUEnd();                                         // завершаем обновления MCU
  }
}

// ********************************************************************** //
// ИНИЦИАЛИЗАЦИЯ РЕЖИМА ПРОГРАММИРОВАНИЯ MCU (сброс MCU, проверка и чтение BIN файлов прошивок, подсчет количества попыток прошивки MCU)
void updateMCUInit() {
  waitMcuUpdateProcess.attach(1, hardResetMCU);             // Ожидаем ответа загрузчика MCU, если не дождемся - сбрасываем физически
  mcuFlashTries++;                                          // Увеличиваем счетчик попыток
  if (mcuFlashTries > 3) {                                  // Если выполнено более трех попыток программирования, то
    updateMCUEnd();                                         // завершаем обновление,
    sendWS(NULL, "infoMessage", "Update Error, check MCU"); // выводим сообщение об ошибке
    return;                                                 // и выходим
  }
  if (SPIFFS.exists(MCU_FLASH)) {                           // Если есть файл прошивки Flash MCU, то
    flagUpdateMCUF = true;                                  // устанавливаем флага обновления Flash
    mcuFlash = SPIFFS.open(MCU_FLASH, "r");                 // открываем файл прошивки Flash MCU на чтение
    mcuFlash.seek(0, SeekSet);                              // переходим на 0 позицию в файле MCUF.bin
  }
  if (SPIFFS.exists(MCU_EEPROM)) {                          // Если есть файл прошивки EEPROM MCU, то
    flagUpdateMCUE = true;                                  // устанавливаем флаг обновления EEPROM
    mcuEEPROM = SPIFFS.open(MCU_EEPROM, "r");               // Открываем файл прошивки EEPROM MCU на чтение
    mcuEEPROM.seek(0, SeekSet);                             // переходим на 0 позицию в файле MCUE.bin
  }
  if (!flagUpdateMCUF && !flagUpdateMCUE) {                 // Если нет ни одного файла прошивки
    sendWS(NULL, "infoMessage", "Select BIN files for MCU");// сообщаем в Web
    return updateMCUEnd();                                  // и завершаем обновления MCU
  }
  sendWS(NULL, "infoMessage", "Reset MCU");                 // Информационные сообщения
  sendUART("SYS", "updateMCU", NULL);                       // Отправляем в MCU команду на перезагрузку
}

// ОПРЕДЕЛЯЕМ УДАЛОСЬ ЛИ СБРОСИТЬ MCU КОМАНДОЙ (сюда попадем через секунду после отправки команды)
void hardResetMCU() {
  if (uartFlowCtrl) {                                       // Если началась прошивка, то
    waitMcuUpdateProcess.detach();                          // выключаем таймер контроля прошивки MCU
  } else {                                                  // Иначе
    sendWS(NULL, "infoMessage", "Hard Reset MCU");          // сообщает о запуске физической перезагрузки MCU,
    digitalWrite(RESET_MCU_PIN, LOW);                       // сбрасываем MCU (через конденсатор),
    waitMcuUpdateProcess.detach();                          // выключаем таймер контроля прошивки MCU,
    waitMcuUpdateProcess.attach(1, updateMCUCrash);         // устанавливаем таймер ожидания ответа после физического сброса на 1 сек
  }
}

// ОПРЕДЕЛЯЕМ УДАЛОСЬ ЛИ СБРОСИТЬ MCU ФИЗИЧЕСКИ (сюда попадем через секунду после импулься сброса)
void updateMCUCrash() {
  if (uartFlowCtrl) {                                       // Если началась прошивка MCU, то
    waitMcuUpdateProcess.detach();                          // останавливаем таймер контроля
  } else {                                                  // Иначе сброс MCU так же не удался, поэтому
    flagUpdateMCU = false;                                  // сбрасываем флаг обновления MCU
    flagUpdateESP = false;                                  // сбрасываем флаг обновления ESP
    flagUpdateALL = false;                                  // сбрасываем флаг обновления всего устройства
    updateMCUEnd();                                         // завершаем обновления MCU
    sendWS(NULL, "infoMessage", "Bootloader MCU Unavailable");
  }
}

// ФИНАЛИЗАЦИЯ ПРОГРАММИРОВАНИЯ MCU
void updateMCUEnd() {
  waitMcuUpdateProcess.detach();                            // Останавливаем таймер контроля прошивки MCU,
  flagUpdate = false;                                       // сбрасываем флаг выполнения какого-либо обновления,
  flagUpdateMCUF = false;                                   // сбрасываем флаг обновления MCU Flash,
  flagUpdateMCUE = false;                                   // сбрасываем флаг обновление MCU EEPROM,
  uartFlowCtrl = false;                                     // запрещаем передачу данных,
  mcuFlashTries = 0;                                        // и устанавливаем счетчик попыток записи в исходное состояние
  mcuFlash.flush(); mcuFlash.close();                       // закрываем его
  mcuEEPROM.flush(); mcuEEPROM.close();                     // закрываем его
  SPIFFS.remove(MCU_FLASH);                                 // Удаляем файл прошивки Flash
  SPIFFS.remove(MCU_EEPROM);                                // Удаляем файл прошивки EEPROM
  digitalWrite(STATUS_LED_PIN, LOW);                        // Гасим светодиод
  digitalWrite(RESET_MCU_PIN, HIGH);                        // Поднимаем уровень на конденсаторе Reset MCU
  if (flagUpdateALL) {                                      // Проверяем, нужно ли обновлять ESP, если да, то
    flagUpdate = true;                                      // активируем признак продолжения обновления
    flagUpdateESP = true;                                   // активируем запуск прошивки ESP
  }
  uartData[0] = '\0';                                       // Чистим буфер UART
  uartMode = UART_NORMAL;                                   // и переводим UART в штатный режим
}

// ********************************************************************** //
// ПРОГРАММИРОВАНИЕ MCU С КОНТРОЛЕМ ПОТОКА
void handlerUARTProg() {
  if (Serial.available()) {

    switch ((char)Serial.read()) {                          // Сравниваем очередной байт из буфера UART
      case BOOT_MCU_XON:                                    // XON
        uartFlowCtrl = true;                                // Разрешаем передачу данных
        break;                                              // Выходим
      case BOOT_MCU_XOFF:                                   // XOFF
        uartFlowCtrl = false;                               // Запрещаем передачу данных
        break;                                              // Выходим
      case BOOT_MCU_SIZE_F:                                 // Превышен размер Flash
      case BOOT_MCU_SIZE_E:                                 // Превышен размер EEPROM
        updateMCUEnd();                                     // прекращаем прошивку MCU
        sendWS(NULL, "infoMessage", "BIN file size Exceeded");  // сообщаем о превышении размера файла прошивки
        break;                                              // Выходим
      case BOOT_MCU_ERROR:                                  // Ошибка записи Flash
        updateMCUInit();                                    // Заново инициализируем режим прошивки MCU
        uartFlowCtrl = false;                               // Запрещаем передачу
        break;                                              // Выходим
      case BOOT_MCU_END:                                    // Удачное завершение прошивки
        updateMCUEnd();                                     // Завершающие действия после обновления прошивки MCU
        sendWS(NULL, "updPrgMCU", "100");                   // Отправляем в WebSocket значение 100 (100% завершено)
        sendWS(NULL, "infoMessage", "MCU Update Complete"); // Отправляем в WebSocket сообщение о завершении
        break;                                              // Выходим
    }
  }

  if (uartFlowCtrl) {                                       // Если передача разрешена, передаем данные
    if (flagUpdateMCUF) {                                   // Если прошиваем MCU Flash
      for (int i = 0; i < sizePackMCU; i++) {               // передаем пакет размером до sizePackMCU
        if (mcuFlash.available())                           // Если не конец файла, то
          Serial.write(mcuFlash.read());                    // передаем очередной байт
        else {                                              // Иначе
          flagUpdateMCUF = false;                           // устанавливаем флаг обновления MCU Flash завершено
          break;                                            // Выхода из for, когда передали блок
        }
      }
      sendWS(NULL, "updPrgMCU", String(mcuFlash.position() * 100 / mcuFlash.size())); // Вычисляем прогресс обновления прошивки MCU и отправляем в WebSocket
    } else if (flagUpdateMCUE) {                            // Если прошиваем MCU EEPROM
      for (int i = 0; i < sizePackMCU; i++) {               // передаем пакет размером до sizePackMCU
        if (mcuEEPROM.available())                          // Если не конец файла, то
          Serial.write(mcuEEPROM.read());                   // передаем очередной байт
        else {                                              // Иначе
          flagUpdateMCUE = false;                           // обновления MCU EEPROM завершено
          break;                                            // Выхода из for, когда передали блок
        }
      }
      sendWS(NULL, "updPrgMCU", String(mcuEEPROM.position() * 100 / mcuEEPROM.size())); // Вычисляем прогресс обновления прошивки MCU и отправляем в WebSocket
    }
  }
  uartFlowCtrl = false;                                     // Пакет передан, запрещаем передачу
  waitMcuUpdateProcess.attach(1, updateMCUEnd);             // Обновляем таймер передачи (выйдем из режима прошивки, по истечению таймера)
}

// ********************************************************************** //
// Проверка файла прошивки MCU
bool checkBin(const char * path) {
  if (!SPIFFS.exists(path) && flagUpdateALL) {              // Если файл прошивки отсутствует и задано обновление всего
    sendWS(NULL, "infoMessage", "BIN doesn't Exist");       // сообщаем об ошибке
    return false;                                           // прерывам обновление
  }

  if (path == MCU_FLASH && flagUpdateALL) {                 // Если обновляем FLASH и задано обновление всего
    if (mcuFlash)                                           // проверяем, открыт ли файл
      mcuFlash.close();                                     // если открыт - закрываем
    mcuFlash = SPIFFS.open(path, "r");                      // Открываем файл на чтение
    if (!mcuFlash) {                                        // Если не удалось открыть
      sendWS(NULL, "infoMessage", "BIN file is Unavailable"); // сообщаем об ошибке
      return false;                                         // прерывам обновление
    }
  }

  if (path == MCU_EEPROM && flagUpdateALL) {                // Если обновляем EEPROM и задано обновление всего
    if (mcuEEPROM)                                          // проверяем, открыт ли файл
      mcuEEPROM.close();                                    // если открыт - закрываем
    mcuEEPROM = SPIFFS.open(path, "r");                     // Открываем файл на чтение
    if (!mcuEEPROM) {                                       // Если не удалось открыть
      sendWS(NULL, "infoMessage", "BIN file is Unavailable"); // сообщаем об ошибке
      return false;                                         // прерывам обновление
    }
  }

  if (flagUpdateMCU && flagUpdateALL) {                     // Если вызвано полное обновление, то
    if (path == MCU_FLASH) {                                // проверяем путь, если путь к FLASH MCU, то
      if (mcuFlash.seek(0, SeekSet)) {                      // проверяем md5 файла прошивки с записью в id.txt с сервера
        MD5Builder md5;                                     // Создаем экземплят мдуля расчета md5
        md5.begin();                                        // Запускаем модуль расчета md5
        md5.addStream(mcuFlash, mcuFlash.size());           // Перебираем файл прошивки
        md5.calculate();                                    // Расчет md5
        if (md5.toString() != md5FlashLatest) {             // Савниваем рассчетное значение md5 и значение из файла id.txt
          mcuFlash.close();                                 // Если не совпадают, то закрываем файл
          sendWS(NULL, "infoMessage", "Invalid MD5");       // и сообщаем пользователю через Web интерфейс
          return false;                                     // md5 не совпадают, обновление отменено
        } else {                                            // Если с md5 все хорошо, то
          mcuFlash.close();                                 // Закрываем файл
          return true;                                      // md5 совпали, продолжаем обновление
        }
      }
    } else if (path == MCU_EEPROM) {                        // проверяем путь, если путь к EEPROM MCU, то
      if (mcuEEPROM.seek(0, SeekSet)) {                     // проверяем md5 файла прошивки с записью в id.txt с сервера
        MD5Builder md5;                                     // Создаем экземплят мдуля расчета md5
        md5.begin();                                        // Запускаем модуль расчета md5
        md5.addStream(mcuEEPROM, mcuEEPROM.size());         // Перебираем файл прошивки
        md5.calculate();                                    // Расчет md5
        if (md5.toString() != md5EEPROMLatest) {            // Савниваем рассчетное значение md5 и значение из файла id.txt
          mcuEEPROM.close();                                // Если не совпадают, то закрываем файл
          sendWS(NULL, "infoMessage", "Invalid MD5");       // и сообщаем пользователю через Web интерфейс
          return false;                                     // md5 не совпадают, обновление отменено
        } else {                                            // Если с md5 все хорошо, то
          mcuEEPROM.close();                                // Закрываем файл
          return true;                                      // md5 совпали, продолжаем обновление
        }
      }
    } else {
      sendWS(NULL, "infoMessage", "Error Path to BIN");     // сообщаем пользователю через Web интерфейс
      return false;
    }
  } else                                                    // Иначе вызвано обновление MCU из фалов
    return true;                                            // проверку md5 не выполняем
  sendWS(NULL, "infoMessage", "Error of BIN file, please reboot the device and try again");
  return false;
}

// ********************************************************************** //
// ОБНОВЛЕНИЕ ESP
void updateESP() {
  sendUART("SYS", "Update ESP", NULL);                      // Сообщаем по UART об обновлении ESP
  flagUpdateESP = false;                                    // Сбрасываем флаг обновления ESP
  flagUpdateALL = false;                                    // Сбрасываем флаг обновления MCU+ESP
  File f = SPIFFS.open(CONFIG_PATH, "r");                   // Читаем файл Конфигурации,
  String fileConfig = f.readString();                       // сохраняем в переменную
  f.flush(); f.close();                                     // и закрываем файл
  ledTicker.detach();                                       // Отключаем моргание
  digitalWrite(STATUS_LED_PIN, HIGH);                       // Зажигаем светодиод
  sendWS(NULL, "infoMessage", "FFS Update...");             // Сообщаем о начале прошивки FFS
  if (_updateFW(true)) {                                    // Обновляем FS
    f = SPIFFS.open(CONFIG_PATH, "w");                      // Востанавливаем файл config.json
    f.print(fileConfig); f.flush(); f.close();              // и закрываем файл
    saveConfig();                                           // Сохраняем текущие настройки в новую FS
    sendWS(NULL, "updPrgFFS", "100");                       // 100% завершение обновления FFS
    sendWS(NULL, "infoMessage", "CORE Update...");          // Сообщаем о начале прошивки ESP
    delay(5);                                               // Задержка
    if (_updateFW(false)) {                                 // Обновляем прошивку модуля
      digitalWrite(STATUS_LED_PIN, LOW);                    // Гасим светодиод
      flagUpdate = false;                                   // Сбрасываем флаг обновления
      sendUART("SYS", "RST", "esp");                        // Отправляем в MCU команду на перезагрузку
      sendWS(NULL, "updPrgESP", "100");                     // 100% завершение обновления CORE
      sendWS(NULL, "infoMessage", "All Updates Completed"); // Сообщаем об успешной прошивке ESP
      delay(20);                                            // Задержка
      ESP.restart();                                        // Перезагрузка, если MCU не перезагрузил ESP
    }
  }
}

// ********************************************************************** //
// ОБНОВЛЕНИЕ FS И ПРОШИВКИ ESP
bool _updateFW(bool updTarget) {
  udp.close();
  HTTPClient httpClient;
  httpClient.useHTTP10(true);
  httpClient.setTimeout(5000);
  char url[65];
  sprintf(url, "http://%s/Updates/%s-%s.bin", OTA_HOST, configDev.di, updTarget ? "EF" : "EC");
  HTTPClient http;
  http.begin(wifiClient, url);
  if (httpClient.GET() != HTTP_CODE_OK) {
    sendWS(NULL, "infoMessage", "Unable to Fetch");
    return false;
  }
  int httpSize = httpClient.getSize();
  if (!Update.setMD5(updTarget ? md5ffsLatest : md5espLatest)) {
    sendWS(NULL, "infoMessage", "Invalid MD5");
    return false;
  }
  if (!Update.begin(httpSize, updTarget ? U_FS : U_FLASH)) {
    sendWS(NULL, "infoMessage", "Incorrect startup conditions");
    return false;
  }
  uint8_t buff[1024] = { 0 };
  size_t sizePack;
  WiFiClient * stream = httpClient.getStreamPtr();
  while (httpClient.connected() && (httpSize > 0 || httpSize == -1)) {
    sizePack = stream->available();
    if (sizePack) {
      int c = stream->readBytes(buff, ((sizePack > sizeof(buff)) ? sizeof(buff) : sizePack));
      Update.write(buff, c);
      if (httpSize > 0)
        httpSize -= c;
    }
    if (progress != int(Update.progress() * 100 / httpClient.getSize())) {
      progress = int(Update.progress() * 100 / httpClient.getSize());
      updTarget ? sendWS(NULL, "updPrgFFS", String(progress)) : sendWS(NULL, "updPrgESP", String(progress));
    }
  }
  if (!Update.end()) {
    sendWS(NULL, "infoMessage", "Invalid completion conditions");
    return false;
  }
  httpClient.end();
  return true;
}
