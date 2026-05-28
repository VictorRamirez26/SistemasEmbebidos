#include <Arduino_FreeRTOS.h>
const int LDR = A3;

void setup() {
    Serial.begin(9600);
    xTaskCreate(TaskLDR, "LDR", 128, NULL, 1, NULL);
}

void loop(){}

void TaskLDR(void *pvParameters){
    while (1){
       int valorLuz = analogRead(LDR);
       Serial.print(valorLuz);
       vTaskDelay(pdMS_TO_TICKS(5000));
    }
}