#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define BTN_START 2
#define BTN_STOP  3

volatile bool running = false;
volatile bool alarm = false;

int lastValue = 0;

SemaphoreHandle_t mutex;
SemaphoreHandle_t semStart;
SemaphoreHandle_t semStop;


void isrStart() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(semStart, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

void isrStop() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(semStop, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}


void TaskMain(void *pvParameters) {
  while (1) {

    // Control por serial (UI)
    if (Serial.available()) {
      char cmd = Serial.read();

      if (cmd == '1') {
        running = true;
      } 
      else if (cmd == '0') {
        running = false;
        alarm = false;
      }
    }

    // Control por botones (interrupciones)
    if (xSemaphoreTake(semStart, 0) == pdPASS) {
      running = true;
    }

    if (xSemaphoreTake(semStop, 0) == pdPASS) {
      running = false;
      alarm = false;
    }

    // Lectura analógica
    if (running) {
      int value = analogRead(A3);

      xSemaphoreTake(mutex, portMAX_DELAY);
      lastValue = value;

      if (value > 800) {
        alarm = true;   // Queda activado
      }

      xSemaphoreGive(mutex);
    }

    // Serial output
    if (running) {
      static TickType_t lastPrint = 0;
      if (xTaskGetTickCount() - lastPrint > pdMS_TO_TICKS(3000)) {

        xSemaphoreTake(mutex, portMAX_DELAY);

        Serial.print("L:");
        Serial.print(lastValue);

        Serial.print(",A:");
        Serial.println(alarm ? 1 : 0);

        xSemaphoreGive(mutex);

        lastPrint = xTaskGetTickCount();
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void TaskLeds(void *pvParameters) {
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  while (1) {
    bool localAlarm;

    xSemaphoreTake(mutex, portMAX_DELAY);
    localAlarm = alarm;
    xSemaphoreGive(mutex);

    if (running) {

      digitalWrite(11, HIGH);
      vTaskDelay(500 / portTICK_PERIOD_MS);
      digitalWrite(11, LOW);
      vTaskDelay(500 / portTICK_PERIOD_MS);

      if (localAlarm) {
        digitalWrite(12, HIGH);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        digitalWrite(12, LOW);
        vTaskDelay(50 / portTICK_PERIOD_MS);
      }

    } else {
      digitalWrite(11, LOW);
      digitalWrite(12, LOW);
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(BTN_START, INPUT);
  pinMode(BTN_STOP, INPUT);

  mutex = xSemaphoreCreateMutex();
  semStart = xSemaphoreCreateBinary();
  semStop  = xSemaphoreCreateBinary();

  attachInterrupt(digitalPinToInterrupt(BTN_START), isrStart, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN_STOP),  isrStop,  FALLING);

  xTaskCreate(TaskMain, "Main", 128, NULL, 2, NULL);
  xTaskCreate(TaskLeds, "Leds", 128, NULL, 1, NULL);
}

void loop() {}