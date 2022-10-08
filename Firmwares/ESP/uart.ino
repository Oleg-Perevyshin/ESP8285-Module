
#include "uart.h"

// ********************************************************************** //
// Инициализация интерфейса UART
void uartInit(int speed) {
  Serial.begin(speed);
  Serial.println();
}

// ********************************************************************** //
// РАБОТАЕМ С UART
void uartWork() {
  switch (uartMode) {
    case UART_NORMAL:
      handlerUARTNormal();
      break;
    case UART_PROG:
      handlerUARTProg();
      break;
    default:
      uartMode = UART_NORMAL;
  }
}

// ********************************************************************** //
// ОБРАБОТКА ПРИНЯТОГО ПАКЕТА ПО UART
void handlerUARTNormal() {
  if (Serial.available()) {                                                                           // Если есть непрочитанные данные в буфере, то
    inChar = (char)Serial.read();                                                                     // читаем очередной символ,
    int uartData_len = strlen(uartData);                                                              // определяем длину пакета
    int uartData_max = sizeof(uartData) - 1;                                                          // определяем максимальную длину буфера
    if (uartData_len < uartData_max) {                                                                // Если буфер не переполнен, то
      uartData[uartData_len] = inChar;                                                                // добавляем очередной считанный байт в буфер,
      uartData[uartData_len + 1] = '\0';                                                              // и закрываем строку
    } else {                                                                                          // В противном случае
      sendUART("ER!", "Buffer", NULL);                                                                // возвращаем ошибку о переполнении буфера
      uartData[0] = '\0';                                                                             // и очищаем его
    }
    if (inChar == EOT) {                                                                              // ОБРАБОТКА ПРИНЯТОГО ПАКЕТА, если пришел символ конца пакета

      if (uartData[0] != SOH) {                                                                       // Если пакет начинается не с символа SOH, то
        sendUART("ER!", "Package", NULL);                                                             // возвращаем ошибку о неверном пакете,
        uartData[0] = '\0';                                                                           // очищаем буфер,
        return;                                                                                       // и выходим
      }

      char header[strchr(uartData + 1, US) - uartData];                                               // ЧИТАЕМ ЗАГОЛОВОК (header)
      strncpy(header, uartData + 1, sizeof(header) - 1);                                              // копируем его в header
      header[sizeof(header) - 1] = '\0';                                                              // и закрываем строку

      char argument[strchr(uartData + 1 + sizeof(header), US) - uartData - sizeof(header)];           // ЧИТАЕМ АРГУМЕНТ (argument)
      strncpy(argument, uartData + sizeof(header) + 1, sizeof(argument) - 1);                         // копируем его в argument
      argument[sizeof(argument) - 1] = '\0';                                                          // и закрываем строку

      bool has_body = strchr(uartData, STX) != NULL;                                                  // ПРОВЕРЯЕМ НАЛИЧИЕ ДАННЫХ В ПАКЕТЕ
      int body_len = has_body ? (strchr(uartData, ETX) - strchr(uartData, STX) - 1) : 0;              // ЧИТАЕМ ДАННЫЕ (body), определяем размер
      char * body = (char*)malloc(body_len + 1);                                                       // выделяем память для данных
      if (body_len)                                                                                   // если есть данные в пакете, то
        strncpy(body, strchr(uartData, STX) + 1, body_len);                                           // копируем их в body
      body[body_len] = '\0';                                                                          // и закрываем строку

      char crc = 0x00;                                                                                // ЧИТАЕМ CRC8, инициализация переменной для хранения рассчитанной контролькой суммы
      if (has_body) {                                                                                 // Если в пакете есть данные, то
        char hex[3];                                                                                  // выделяем массив символов
        strncpy(hex, strchr(uartData, ETX) + 1, 2);                                                   // читаем CRC8 из пакета в массив
        hex[2] = '\0';                                                                                // закрываем строку
        crc = (char)strtol(hex, NULL, 16);                                                            // преобразуем формат
      }

      if (has_body && crc8(body, body_len) != crc) {                                                  // СРАВНИВАЕМ CRC8, если расчитанная контрольная сумма не соответствует контрольной сумме в пакете, то
        sendUART("ER!", "CRC8", NULL);                                                                // Возвращаем ошибку,
        free(body);                                                                                   // освобождаем память,
        uartData[0] = '\0';                                                                           // очищаем буфер,
        return;                                                                                       // и выходим
      }

      // Обрабатываем зоголовки
      if (!strcmp(header, "SYS"))
        parserSYS(argument, body);
      else if (!strcmp(header, "GET"))
        parserGET(argument);
      else if (!strcmp(header, "SET"))
        parserSET(argument, body);
      else if (!strcmp(header, "OK!"))
        parserOK(argument, body);
      else if (!strcmp(header, "ER!"))
        parserER(argument, body);
      else
        sendUART("ER!", "Header", NULL);

      free(body);
      uartData[0] = '\0';
    }
  }
}

// ********************************************************************** //
// Передача пакета в UART
void sendUART(const char * header, const char * argument, const char * value) {
  if (uartMode == UART_PROG)
    return;
  digitalWrite(STATUS_LED_PIN, HIGH);               // Зажигаем светодиод
  if (value)
    Serial.printf( "%c%s%c%s%c%c%s%c%02X%c", SOH, header, US, argument, US, STX, value, ETX, crc8(value, strlen(value)), EOT );
  else
    Serial.printf( "%c%s%c%s%c%c", SOH, header, US, argument, US, EOT );
  delay(10);
  digitalWrite(STATUS_LED_PIN, LOW);                // Гасим светодиод
}

// ********************************************************************** //
// Расчет контрольной суммы CRC8
byte crc8(const char * value, size_t len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *value++;
    for (byte i = 8; i; i--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum)
        crc ^= 0x8C;
      extract >>= 1;
    }
  }
  return crc;
}
