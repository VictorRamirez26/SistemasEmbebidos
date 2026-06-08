#include <EEPROM.h>

#define TRIG_PIN 5
#define ECHO_PIN 4

const byte leds[] = {6, 7, 8, 9, 10, 11, 12, 13};

const float DISTANCIA_UMBRAL = 50.0;

const unsigned long TIEMPO_MINIMO = 500;
const unsigned long TIEMPO_ALARMA = 5000;

unsigned long inicioDeteccion = 0;

bool personaPresente = false;
bool personaValida = false;
bool alarma = false;

unsigned int contadorPersonas = 0;

String bufferSerie = "";

float medirDistancia();
void guardarContador();
void cargarContador();
void enviarEstado();

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  for (byte i = 0; i < 8; i++) {
    pinMode(leds[i], OUTPUT);
  }

  cargarContador();

  enviarEstado();
}

void loop() {

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      bufferSerie.trim();

      if (bufferSerie == "RESET") {
        contadorPersonas = 0;
        guardarContador();
        enviarEstado();
      }

      bufferSerie = "";
    }
    else {
      bufferSerie += c;
    }
  }

  float distancia = medirDistancia();

  static float anterior = distancia;
  static float anterior2 = distancia;

  bool lecturaIncoherente =
    abs(distancia - anterior) > 40 &&
    abs(anterior2 - anterior) > 40;

  if (lecturaIncoherente) {
    distancia = anterior;
  }

  anterior2 = anterior;
  anterior = distancia;

  bool detectado = distancia < DISTANCIA_UMBRAL;

  if (!personaPresente && detectado) {

    personaPresente = true;
    personaValida = false;
    alarma = false;

    inicioDeteccion = millis();
  }

  if (personaPresente) {

    unsigned long tiempo = millis() - inicioDeteccion;

    if (tiempo >= TIEMPO_MINIMO) {
      personaValida = true;
    }

    if (tiempo >= TIEMPO_ALARMA) {
      alarma = true;
    }

    if (!detectado) {

      if (personaValida) {

        contadorPersonas++;

        guardarContador();

        Serial.println("EVENTO");
      }

      personaPresente = false;
      personaValida = false;
      alarma = false;

      enviarEstado();
    }
  }

  for (byte i = 0; i < 8; i++) {
    digitalWrite(leds[i], alarma);
  }

  static unsigned long ultimoEnvio = 0;

  if (millis() - ultimoEnvio > 1000) {
    ultimoEnvio = millis();
    enviarEstado();
  }

  delay(50);
}

float medirDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) {
    return 400;
  }

  return duracion * 0.0343 / 2.0;
}

void guardarContador() {
  EEPROM.put(0, contadorPersonas);
}

void cargarContador() {
  EEPROM.get(0, contadorPersonas);

  if (contadorPersonas > 50000) {
    contadorPersonas = 0;
  }
}

void enviarEstado() {

  Serial.print("CONTADOR:");
  Serial.print(contadorPersonas);

  Serial.print(";ALARMA:");
  Serial.println(alarma ? 1 : 0);
}