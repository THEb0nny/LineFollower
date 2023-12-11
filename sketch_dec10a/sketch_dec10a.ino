// https://www.youtube.com/watch?v=gm6VetxM3lU
// https://www.youtube.com/watch?v=8Lj5ycrT9Fw
// https://www.youtube.com/watch?v=VjdcSfm9kGc
// https://github.com/GyverLibs/TimerMs
// https://alexgyver.ru/gyverpid/
// https://github.com/GyverLibs/GyverPID
// https://github.com/pololu/qtr-sensors-arduino

#include <QTRSensors.h>
#include <TimerMs.h>

#define DEBUG_LINE_SENSOR_RAW_VALUES true // Печать для дебага сырых значений с датчика линии при калибровки
#define DEBUG_LINE_SENSOR_VALUES true // Печать нормализованных значений с сенсоров датчика линии

#define QTR_SEN_COUNT 8 // Количество сенсоров в датчике линии
#define QTR_IR_PIN 12
#define QTR_D1_PIN A0
#define QTR_D2_PIN A1
#define QTR_D3_PIN A2
#define QTR_D4_PIN A3
#define QTR_D5_PIN A4
#define QTR_D6_PIN A5
#define QTR_D7_PIN A6
#define QTR_D8_PIN A7

#define MAX_RANGE_VAl_LS 255 // Максимальное значенение диапазона для нормализации значений сенсоров датчика линии

QTRSensors qtr; // Создаём объект датчика линии
TimerMs tmrPrint(10); // Объект таймера для печати инфы с сенсоров датчика линии в интервале

byte qtr8aPins[QTR_SEN_COUNT] = {QTR_D1_PIN, QTR_D2_PIN, QTR_D3_PIN, QTR_D4_PIN, QTR_D5_PIN, QTR_D6_PIN, QTR_D7_PIN, QTR_D8_PIN};  // Массив пинов сенсоров датчика линии

unsigned int qtrSensorValues[QTR_SEN_COUNT]; // Массив для хранения нормализованных значений с сенсоров датчика линии

void setup() {
  Serial.begin(115200); // Устанавливаем скорость общения по Serial
  Serial.setTimeout(5); // Позволяет задать время ожидания данных
  Serial.println();
  Serial.println("Setup"); // Сообщение о конце инициализации
  qtr.setTypeAnalog(); // Установить тип аналогового датчика линии
  qtr.setSensorPins(qtr8aPins, QTR_SEN_COUNT); // Передаём пины и устанавливаем количество сенсоров в датчики линии
  qtr.setEmitterPin(QTR_IR_PIN); // Подключаем IR подсветку
  pinMode(LED_BUILTIN, OUTPUT); // Установить режим пина встроенного светодиода
  digitalWrite(LED_BUILTIN, HIGH); // Включаем встроенный светодиод для сигнала начала калибровки
  Serial.println("Calibrate sensors start"); // Сообщение о конце калибровки
  for (byte i = 0; i < 200; i++) {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // Выключения встроенный светодиод для сигнала о завершении калибровки
  Serial.println("Calibrate done"); // Сообщение о конце калибровки
  if (DEBUG_LINE_SENSOR_RAW_VALUES) { // Печатать калибровочные данные белого и чёрного
    for (byte i = 0; i < QTR_SEN_COUNT; i++) { // Печатать минимальные значения при включенном эмиттере
      if (i < QTR_SEN_COUNT - 1) Serial.print(String(qtr.calibrationOn.minimum[i]) + " ");
      else Serial.println(String(qtr.calibrationOn.maximum[i]));
    }
    Serial.println();
    for (byte i = 0; i < QTR_SEN_COUNT; i++) { // Печатать максимальные значения при включенном эмиттере
      if (i < QTR_SEN_COUNT - 1) Serial.print(String(qtr.calibrationOn.maximum[i]) + " ");
      else Serial.println(String(qtr.calibrationOn.maximum[i]));
    }
  }
  Serial.println("Initial completed"); // Сообщение о конце инициализации
  tmrPrint.setPeriodMode(); // Установить в режиме периода таймер печати
  tmrPrint.start(); // Запускаем таймер печати в режиме интервала
}

void loop() {
  unsigned int position = qtr.readLineBlack(qtrSensorValues); // Прочитать значения с сенсоров датчика линии и получить позицию, записав их в qtrSensorValues
  // qtr.read(qtrSensorValues); // Считать с датчика значения
  if (DEBUG_LINE_SENSOR_VALUES && tmrPrint.tick()) {
    for (byte i = 0; i < QTR_SEN_COUNT; i++) {
      Serial.print(qtrSensorValues[i]);
      Serial.print('\t');
    }
    Serial.println(position);
  }
}