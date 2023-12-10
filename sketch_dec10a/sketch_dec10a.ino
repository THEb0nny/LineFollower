#include <TimerMs.h>

#define DEBUG_LINE_SENSOR_RAW_VALUES true // Печать для дебага сырые значения с сенсоров датчика линии
#define DEBUG_LINE_SENSOR_VALUES true // Печать нормализованных значений с сенсоров датчика линии

#define QTR_8A_PINS_COUNT 8 // Количество сенсоров в датчике линии
#define QTR_8A_IR_PIN 5
#define QTR_8A_D1_PIN A0
#define QTR_8A_D2_PIN A1
#define QTR_8A_D3_PIN A2
#define QTR_8A_D4_PIN A3
#define QTR_8A_D5_PIN A4
#define QTR_8A_D6_PIN A5
#define QTR_8A_D7_PIN A6
#define QTR_8A_D8_PIN A7

#define MAX_RANGE_VAl_LS 255 // Максимальное значенение диапазона для нормализации значений сенсоров датчика линии

TimerMs tmrPrint(10); // Таймер для печати инфы с сенсоров датчика линии в интервале

byte qtr8aPins[QTR_8A_PINS_COUNT] = {QTR_8A_D1_PIN, QTR_8A_D2_PIN, QTR_8A_D3_PIN, QTR_8A_D4_PIN, QTR_8A_D5_PIN, QTR_8A_D6_PIN, QTR_8A_D7_PIN, QTR_8A_D8_PIN}; // Массив пинов сенсоров датчика линии

int qtr8aSensorsBlackValues[QTR_8A_PINS_COUNT] = {830, 830, 830, 840, 830, 830, 830, 830}; // Сырые значения сенсоров датчика линии на чёрном
int qtr8aSensorsWhiteValues[QTR_8A_PINS_COUNT] = {1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010}; // Сырые значения сенсоров датчика линии на белом

int qtr8aSensorsRawValues[QTR_8A_PINS_COUNT]; // Массив для хранения сырых значений с сенсоров датчика линии
int qtr8aSensorsValues[QTR_8A_PINS_COUNT]; // Массив для хранения нормализованных значений с сенсоров датчика линии

void setup() {
  Serial.begin(115200); // Устанавливаем скорость общения по Serial
  Serial.setTimeout(5); // Позволяет задать время ожидания данных
  for (byte i = 0; i < QTR_8A_PINS_COUNT; i++) { // Настройка пинов сенсоров на датчике линии
    pinMode(qtr8aPins[i], INPUT);
  }
  Serial.println("Initial completed"); // Сообщение о конце инициализации
  tmrPrint.setPeriodMode(); // Установить в режиме периода таймер печати
  tmrPrint.start(); // Запускаем таймер печати в режиме интервала
}

void loop() {
  ReadLineSensorValues(); // Считать сырые значения с сенсоров датчика линии
  NormLineSensorValues(); // Нормализовать сырые значения с сенсоров датчика линии
  if (tmrPrint.tick()) PrintValuesFromLineSensorForDebug();
}

// Функция для считывания сырых значений сенсоров с датчика линии
void ReadLineSensorValues() {
  for (byte i = 0; i < QTR_8A_PINS_COUNT; i++) { // Считываем со всех сенсоров датчика линии и записываем в массив
    qtr8aSensorsRawValues[i] = analogRead(qtr8aPins[i]);
  }
}

// Функция для нормализации значений с сенсоров датчика линии
void NormLineSensorValues() {
  for (byte i = 0; i < QTR_8A_PINS_COUNT; i++) {
    qtr8aSensorsValues[i] = map(qtr8aSensorsRawValues[i], qtr8aSensorsWhiteValues[i], qtr8aSensorsBlackValues[i], 0, MAX_RANGE_VAl_LS); // Перевод сырого значения в новый диапазон
    qtr8aSensorsValues[i] = constrain(qtr8aSensorsValues[i], 0, MAX_RANGE_VAl_LS); // Ограничить значение для защиты выхода за диапазон
  }
}

// Функция для печати значений с датчика линии
void PrintValuesFromLineSensorForDebug() {
  if (DEBUG_LINE_SENSOR_RAW_VALUES) { // Если нужно печатать сырые значения с сенсоров датчика линии
    for (byte i = 0; i < QTR_8A_PINS_COUNT; i++) {
      if (i < QTR_8A_PINS_COUNT - 1 || DEBUG_LINE_SENSOR_VALUES) Serial.print(String(qtr8aSensorsRawValues[i]) + "\t");
      else Serial.println(qtr8aSensorsRawValues[i]);
    }
  }
  if (DEBUG_LINE_SENSOR_VALUES) { // Если нужно печатать нормализованные значения с сенсоров датчика линии
    for (byte i = 0; i < QTR_8A_PINS_COUNT; i++) {
      if (i < QTR_8A_PINS_COUNT - 1) Serial.print(String(qtr8aSensorsValues[i]) + "\t");
      else Serial.println(qtr8aSensorsValues[i]);
    }
  }
}