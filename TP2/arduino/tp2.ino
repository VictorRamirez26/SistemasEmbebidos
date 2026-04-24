#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define BTN_START 2
#define BTN_STOP  3

int lastValue = 0;
volatile bool running = false;
SemaphoreHandle_t mutex;

void TaskButtons(void *pvParameters) {
  pinMode(BTN_START, INPUT); // pull-down externo
  pinMode(BTN_STOP, INPUT);

  bool lastStartState = LOW;
  bool lastStopState = LOW;

  while (1) {
    bool currentStart = digitalRead(BTN_START);
    bool currentStop  = digitalRead(BTN_STOP);

    // Detectar flanco de subida (LOW → HIGH)
    if (currentStart == HIGH && lastStartState == LOW) {
      running = true;
    }

    if (currentStop == HIGH && lastStopState == LOW) {
      running = false;
    }

    lastStartState = currentStart;
    lastStopState  = currentStop;

    vTaskDelay(50 / portTICK_PERIOD_MS); // pequeño debounce
  }
}

void TaskSerialControl(void *pvParameters) {
  while (1) {
    if (Serial.available()) {
      char cmd = Serial.read();

      if (cmd == '1') {
        running = true;   // START
      } 
      else if (cmd == '0') {
        running = false;  // STOP
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void TaskAnalog(void *pvParameters) {
  while (1) {

    if (running) {
      int value = analogRead(A3);

      xSemaphoreTake(mutex, portMAX_DELAY);
      lastValue = value;
      xSemaphoreGive(mutex);
    }

    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void TaskSerial(void *pvParameters) {
  while (1) {
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    if (running) {
      xSemaphoreTake(mutex, portMAX_DELAY);

      Serial.println(lastValue);

      xSemaphoreGive(mutex);
    }
  }
}

void TaskBlink(void *pvParameters) {
  pinMode(11, OUTPUT);

  while (1) {
    if (running) {
      digitalWrite(11, HIGH);
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      digitalWrite(11, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else {
      digitalWrite(11, LOW); // asegurarse que esté apagado
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void setup() {
  Serial.begin(9600);

  mutex = xSemaphoreCreateMutex();

  xTaskCreate(TaskAnalog, "Analog", 128, NULL, 1, NULL);
  xTaskCreate(TaskSerial, "Serial", 128, NULL, 1, NULL);
  xTaskCreate(TaskButtons, "Buttons", 128, NULL, 1, NULL);
  xTaskCreate(TaskSerialControl, "SerialCtrl", 128, NULL, 1, NULL);
  xTaskCreate(TaskBlink, "Blink", 128, NULL, 1, NULL);
}

void loop() {}