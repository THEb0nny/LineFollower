// https://www.youtube.com/watch?v=gm6VetxM3lU
// https://www.youtube.com/watch?v=8Lj5ycrT9Fw
// https://www.youtube.com/watch?v=VjdcSfm9kGc
// https://github.com/GyverLibs/TimerMs
// https://alexgyver.ru/gyverpid/
// https://github.com/GyverLibs/GyverPID
// https://github.com/pololu/qtr-sensors-arduino

#define PID_OPTIMIZED_I // Параметр для оптимизации суммы регулятора

#include <QTRSensors.h>
#include "GyverMotor2.h"
#include "GyverPID.h"
#include <TimerMs.h>
#include <EncButton.h>

#define DEBUG_LINE_SENSOR_RAW_VALUES true // Печать для дебага сырых значений с датчика линии при калибровки
#define DEBUG_LINE_SENSOR_VALUES true // Печать нормализованных значений с сенсоров датчика линии
#define PRINT_DT_ERR_U_DEBUG false // Печать информации о loopTime, error, u TRUE

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

GMotor2<DRIVE3WIRE> leftMotor(7, 8, 9);
GMotor2<DRIVE3WIRE> righMotor(5, 4, 3);

QTRSensors qtr; // Создаём объект датчика линии
TimerMs tmrPrint(10); // Объект таймера для печати инфы с сенсоров датчика линии в интервале

byte qtr8aPins[QTR_SEN_COUNT] = {QTR_D1_PIN, QTR_D2_PIN, QTR_D3_PIN, QTR_D4_PIN, QTR_D5_PIN, QTR_D6_PIN, QTR_D7_PIN, QTR_D8_PIN};  // Массив пинов сенсоров датчика линии

unsigned int qtrSensorValues[QTR_SEN_COUNT]; // Массив для хранения нормализованных значений с сенсоров датчика линии

void(* softResetFunc) (void) = 0; // Функция мягкого перезапуска

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
  regulator.setDirection(REVERSE); // Направление регулирования (NORMAL/REVERSE)
  regulator.setLimits(-200, 200); // Пределы регулятора
  regulatorTmr.setPeriodMode(); // Настроем режим условия регулирования на период
  motorLeft.reverse(1); // Реверс левого мотора
  motorRight.reverse(0); // Реверс правого мотора
  motorLeft.setMinDuty(10); // Мин ШИМ левого мотора
  motorRight.setMinDuty(10); // Мин ШИМ правого мотора
  regulatorTmr.setPeriodMode(); // Настроем режим условия регулирования на период
  Serial.println("Initial completed"); // Сообщение о конце инициализации
  while (millis() < 500); // Время после старта для возможности запуска, защита от перезагрузки и старта кода сразу
  Serial.println("Ready... press btn");
  while (true) { // Ждём нажатие кнопки для старта
    btn.tick(); // Опрашиваем кнопку
    if (btn.press()) { // Произошло нажатие
      Serial.println("Go!!!");
      break;
    }
  }
  regulatorTmr.start(); // Запускаем таймер цикла регулирования
  // Записываем время перед стартом loop
  currTime = millis();
  prevTime = currTime;
}

void loop() {
  CheckBtnClick(); // Вызываем функцию опроса кнопки

  if (regulatorTmr.tick()) { // Раз в 10 мсек выполнять
    currTime = millis();
    loopTime = currTime - prevTime;
    prevTime = currTime;
    unsigned int position = qtr.readLineBlack(qtrSensorValues); // Прочитать значения с сенсоров датчика линии и получить позицию, записав их в qtrSensorValues
    // qtr.read(qtrSensorValues); // Считать с датчика значения
    CheckBtnClick(); // Вызываем функцию опроса кнопки

    float error = 0;
    regulator.setpoint = error; // Передаём ошибку
    regulator.setDt(loopTime != 0 ? loopTime : 1); // Установка dt для регулятора
    float u = regulator.getResult(); // Управляющее воздействие с регулятора

    MotorsControl(u, 128); // Для управления моторами регулятором

    if (DEBUG_LINE_SENSOR_VALUES && tmrPrint.tick()) {
      for (byte i = 0; i < QTR_SEN_COUNT; i++) {
        Serial.print(qtrSensorValues[i]);
        Serial.print('\t');
      }
      Serial.println(position);
    }

    // Для отладки основной информации о регулировании
    if (PRINT_DT_ERR_U_DEBUG) {
      Serial.print("loopTime: " + String(loopTime) + "\t");
      Serial.print("error: " + String(error) + "\t");
      Serial.println("u: " + String(u));
    }
  }
}

// Управление двумя моторами
void MotorsControl(int dir, int speed) {
  int lMotorSpeed = speed + dir, rMotorSpeed = speed - dir;
  float z = (float) speed / max(abs(lMotorSpeed), abs(rMotorSpeed)); // Вычисляем отношение желаемой мощности к наибольшей фактической
  lMotorSpeed *= z, rMotorSpeed *= z;
  motorLeft.setSpeed(lMotorSpeed);
  motorRight.setSpeed(rMotorSpeed);
}

// Функция опроса о нажатии кнопки
void CheckBtnClick() {
  btn.tick(); // Опрашиваем кнопку в первый раз
  if (btn.press()) { // Произошло нажатие
    Serial.println("Btn press and reset");
    delay(50); // Нужна задержка иначе не выведет сообщение
    softResetFunc(); // Если клавиша нажата, то сделаем мягкую перезагрузку
  }
}