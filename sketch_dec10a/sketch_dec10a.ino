// https://www.youtube.com/watch?v=gm6VetxM3lU
// https://www.youtube.com/watch?v=8Lj5ycrT9Fw
// https://www.youtube.com/watch?v=VjdcSfm9kGc
// https://github.com/GyverLibs/TimerMs
// https://alexgyver.ru/gyverpid/
// https://github.com/GyverLibs/GyverPID
// https://github.com/pololu/qtr-sensors-arduino

#include <QTRSensors.h>
#include <TimerMs.h>

#define DEBUG_LINE_SENSOR_RAW_VALUES true // Печать для дебага сырые значения с сенсоров датчика линии
#define DEBUG_LINE_SENSOR_VALUES true // Печать нормализованных значений с сенсоров датчика линии

#define QTR_SEN_COUNT 8 // Количество сенсоров в датчике линии
#define QTR_IR_PIN 5
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
TimerMs tmrPrint(50); // Объект таймера для печати инфы с сенсоров датчика линии в интервале

byte qtr8aPins[QTR_SEN_COUNT] = {QTR_D1_PIN, QTR_D2_PIN, QTR_D3_PIN, QTR_D4_PIN, QTR_D5_PIN, QTR_D6_PIN, QTR_D7_PIN, QTR_D8_PIN}; // Массив пинов сенсоров датчика линии

unsigned int qtrSensorsBlackValues[QTR_SEN_COUNT] = {830, 830, 830, 840, 830, 830, 830, 830}; // Сырые значения сенсоров датчика линии на чёрном
unsigned int qtrSensorsWhiteValues[QTR_SEN_COUNT] = {1010, 1010, 1010, 1010, 1010, 1010, 1010, 1010}; // Сырые значения сенсоров датчика линии на белом

unsigned int qtrSensorValues[QTR_SEN_COUNT]; // Массив для хранения нормализованных значений с сенсоров датчика линии

void setup() {
  Serial.begin(115200); // Устанавливаем скорость общения по Serial
  Serial.setTimeout(5); // Позволяет задать время ожидания данных
  qtr.setTypeAnalog(); // Установить тип аналогового датчика линии
  qtr.setSensorPins(qtr8aPins, QTR_SEN_COUNT); // Передаём пины и устанавливаем количество сенсоров в датчики линии
  qtr.setEmitterPin(QTR_IR_PIN); // Не понял что это, пин IR?
  delay(500); // Задержка из примера, похоже для инициализации датчика линии
  pinMode(LED_BUILTIN, OUTPUT); // Установить режим пина встроенного светодиода
  digitalWrite(LED_BUILTIN, HIGH); // Включаем встроенный светодиод для сигнала начала калибровки
  for (byte i = 0; i < 400; i++) {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // Выключения встроенный светодиод для сигнала о завершении калибровки
  for (byte i = 0; i < QTR_SEN_COUNT; i++) { // print the calibration minimum values measured when emitters were on
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();
  for (byte i = 0; i < QTR_SEN_COUNT; i++) { // print the calibration maximum values measured when emitters were on
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println("Initial completed"); // Сообщение о конце инициализации
  tmrPrint.setPeriodMode(); // Установить в режиме периода таймер печати
  tmrPrint.start(); // Запускаем таймер печати в режиме интервала
}

void loop() {
  unsigned int position = qtr.readLineBlack(qtrSensorValues);
  //qtr.read(qtrSensorValues); // Считать с датчика сырые значения
  if (tmrPrint.tick()) {
    Serial.println(position);
    for (byte i = 0; i < QTR_SEN_COUNT; i++) {
      Serial.print(qtrSensorValues[i]);
      Serial.print('\t');
    }
  }
}