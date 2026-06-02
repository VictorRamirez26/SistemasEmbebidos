#include <Arduino.h>

const int TRIG_PIN = 5;
const int ECHO_PIN = 4;

const int LED_ENTRADA = 10;
const int LED_SALIDA = 11;
const int LED_ALARMA = 12;

int altura_tanque = 100;

int nivelMinimo = 20;
int nivelMaximo = 80;

unsigned long ultimoEnvio = 0;

String buffer = "";

float medirDistancia();
void procesarMensaje(String msg);

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_ENTRADA, OUTPUT);
  pinMode(LED_SALIDA, OUTPUT);
  pinMode(LED_ALARMA, OUTPUT);

  digitalWrite(LED_ENTRADA, LOW);
  digitalWrite(LED_SALIDA, LOW);
  digitalWrite(LED_ALARMA, LOW);
}

void loop() {

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      procesarMensaje(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }

  float distancia = medirDistancia();

  int nivel = altura_tanque - distancia;

  if (nivel < 0) {
    nivel = 0;
  }

  if (nivel > altura_tanque) {
    nivel = altura_tanque;
  }

  String estado = "NORMAL";

  if (nivel < nivelMinimo) {

    digitalWrite(LED_ENTRADA, HIGH);
    digitalWrite(LED_SALIDA, LOW);
    digitalWrite(LED_ALARMA, HIGH);

    estado = "LOW";

  } else if (nivel > nivelMaximo) {

    digitalWrite(LED_ENTRADA, LOW);
    digitalWrite(LED_SALIDA, HIGH);
    digitalWrite(LED_ALARMA, HIGH);

    estado = "HIGH";

  } else {

    digitalWrite(LED_ENTRADA, LOW);
    digitalWrite(LED_SALIDA, LOW);
    digitalWrite(LED_ALARMA, LOW);

    estado = "NORMAL";
  }

  if (millis() - ultimoEnvio >= 1000) {

    ultimoEnvio = millis();

    Serial.print("LEVEL:");
    Serial.print(nivel);

    Serial.print(";STATUS:");
    Serial.print(estado);

    Serial.print(";MIN:");
    Serial.print(nivelMinimo);

    Serial.print(";MAX:");
    Serial.print(nivelMaximo);

    Serial.print(";ALT:");
    Serial.println(altura_tanque);
  }

  delay(100);
}

float medirDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) {
    return altura_tanque;
  }

  float distancia = duracion * 0.0343 / 2.0;

  return distancia;
}

void procesarMensaje(String msg) {

  msg.trim();

  int inicio = 0;

  while (inicio < msg.length()) {

    int fin = msg.indexOf(';', inicio);

    if (fin == -1) {
      fin = msg.length();
    }

    String campo = msg.substring(inicio, fin);

    if (campo.startsWith("MIN:")) {

      nivelMinimo = campo.substring(4).toInt();

    }
    else if (campo.startsWith("MAX:")) {

      nivelMaximo = campo.substring(4).toInt();

    }
    else if (campo.startsWith("ALT:")) {

      altura_tanque = campo.substring(4).toInt();

    }

    inicio = fin + 1;
  }

}
