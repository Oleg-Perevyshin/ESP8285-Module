
#ifndef _UPDATE_H_
#define _UPDATE_H_


// ПУТИ
#define OTA_HOST          "soi-tech.com"    // Адрес сервера обновлений
#define MCU_FLASH         "/MF.bin"         // Путь к файлу прошивки MCU Flash
#define MCU_EEPROM        "/ME.bin"         // Путь к файлу прошивки MCU EEPROM

// ФЛАГИ ОБНОВЛЕНИЯ
bool flagUpdate =         false;            // Запуск обновления
bool flagUpdateMCU =      false;            // Запуск обновление прошивки MCU
bool flagUpdateMCUF =     false;            // Запуск обновления прошивки MCU Flash
bool flagUpdateMCUE =     false;            // Запуск обновления прошивки MCU EEPROM
bool flagUpdateESP =      false;            // Запуск обновления прошивки ESP
bool flagUpdateALL =      false;            // Запуск обновления прошивок MCU + ESP

// ФАЙЛЫ ПРОШИВОК
File mcuFlash;                              // Переменная для хранения Flash MCU
File mcuEEPROM;                             // Переменная для хранения EEPROM MCU

// УПРАВЛЕНИЕ ПОТОКОМ И КОНТРОЛЬ
bool uartFlowCtrl =       false;            // Управление потоком в режиме прошивки MCU
int mcuFlashTries =       0;                // Счетчик попыток записи во флешь MCU
int progress =            0;                // Прогресс обновления в %
#define BOOT_MCU_XON      0x11              // Управление потоком, передача разрешена
#define BOOT_MCU_XOFF     0x13              // Управление потоком, передача запрещена
#define BOOT_MCU_SIZE_F   'F'               // Превышен размер файла прошивки MCU Flash
#define BOOT_MCU_SIZE_E   'E'               // Превышен размер файла прошивки MCU EEPROM
#define BOOT_MCU_ERROR    '#'               // Ошибка записи во Flash
#define BOOT_MCU_END      '<'               // Удачное завершение прошивки

// РАЗМЕРЫ ПОЛУЧЕННЫЕ ОТ MCU
int sizeFlashMCU =        0;                // Размер Flash MCU
int sizePackMCU =         0;                // Размер передаваемых блоков
int sizeEEPROMMCU =       0;                // Размер EEPROM MCU

// ФУНКЦИИ
void updateMCUInit();                       // Инициализация прошивки MCU (перезагрузка и ожидание ответа загрузчика)
void handlerUARTProg();                     // Работа UART в режиме программирования MCU
void updateMCUEnd();                        // Финализация
bool checkBin(const char * path);           // Проверка контрольной суммы файла по пути path
void updateMCU();                           // Начало прошивки MCU (проверка bin файла)
void updateESP();                           // Обновление прошивки ESP:
                                            // - сохранение config.json
                                            // - обновление файловой системы
                                            // - восстанавление config.json
                                            // - обновление ядра


#endif
