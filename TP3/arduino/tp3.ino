#include <Arduino.h>
#include <EEPROM.h>

unsigned long unixTime = 0;

unsigned long ultimoIncremento = 0;

String buffer = "";

volatile bool eventoPin2 = false;
volatile bool eventoPin3 = false;

volatile unsigned long ultimoRebotePin2 = 0;
volatile unsigned long ultimoRebotePin3 = 0;

// bytes 0 y 1:
// guardan la próxima dirección libre

uint16_t proximaDireccion = 2;

void procesarMensaje(String msg);

void guardarEvento(byte evento);

void interrupcionPin2();

void interrupcionPin3();

void leerEventos();

void limpiarEEPROM();

void setup() {

  Serial.begin(9600);

  pinMode(2, INPUT);
  pinMode(3, INPUT);

  attachInterrupt(
    digitalPinToInterrupt(2),
    interrupcionPin2,
    FALLING
  );

  attachInterrupt(
    digitalPinToInterrupt(3),
    interrupcionPin3,
    FALLING
  );

  // Recupero puntero eeprom
  EEPROM.get(0, proximaDireccion);

  // validar puntero
  if (
    proximaDireccion < 2 ||
    proximaDireccion > EEPROM.length()
  ) {

    proximaDireccion = 2;
  }

  Serial.println("ARDUINO_LISTO");
}

void loop() {

  // Unix
  if (millis() - ultimoIncremento >= 1000) {

    unixTime++;

    ultimoIncremento += 1000;
  }

  while (Serial.available()) {

    char c = Serial.read();

    if (c == '\n') {

      procesarMensaje(buffer);

      buffer = "";
    }
    else {

      buffer += c;
    }
  }

  if (eventoPin2) {

    guardarEvento(2);

    eventoPin2 = false;
  }

  if (eventoPin3) {

    guardarEvento(3);

    eventoPin3 = false;
  }
}


void leerEventos() {

  int address = 2;

  while (address + 5 <= proximaDireccion) {

    unsigned long timestamp;

    byte evento;

    EEPROM.get(address, timestamp);

    evento = EEPROM.read(address + 4);

    // validar evento
    if (evento != 2 && evento != 3) {
      break;
    }

    Serial.print("EVENT:");

    Serial.print(timestamp);

    Serial.print(",");

    Serial.println(evento);

    address += 5;
  }

  Serial.println("END_EVENTS");
}

void limpiarEEPROM() {

  for (int i = 0; i < EEPROM.length(); i++) {

    EEPROM.write(i, 0xFF);
  }

  proximaDireccion = 2;

  EEPROM.put(0, proximaDireccion);

  Serial.println("EEPROM_CLEARED");
}

void procesarMensaje(String msg) {

  msg.trim();

  if (msg.startsWith("TIME:")) {

    String valor = msg.substring(5);

    unixTime = valor.toInt();

    Serial.println("OK_TIME_UPDATED");
  }

  else if (msg == "GETTIME") {

    Serial.print("TIME:");

    Serial.println(unixTime);
  }

  else if (msg == "GETEVENT") {

    leerEventos();
  }

  else if (msg == "CLEAREEPROM") {

    limpiarEEPROM();
  }

  else {

    Serial.println("ERROR_UNKNOWN_COMMAND");
  }
}

void guardarEvento(byte evento) {

  // verificar espacio
  if (proximaDireccion + 5 >= EEPROM.length()) {

    Serial.println("EEPROM_FULL");

    return;
  }

  EEPROM.put(proximaDireccion, unixTime);

  EEPROM.write(
    proximaDireccion + 4,
    evento
  );

  proximaDireccion += 5;

  // guardar nuevo puntero
  EEPROM.put(0, proximaDireccion);

  Serial.println("EVENT_SAVED");
}

void interrupcionPin2() {

  unsigned long ahora = millis();

  // debounce
  if (ahora - ultimoRebotePin2 > 200) {

    eventoPin2 = true;

    ultimoRebotePin2 = ahora;
  }
}

void interrupcionPin3() {

  unsigned long ahora = millis();

  // debounce
  if (ahora - ultimoRebotePin3 > 200) {

    eventoPin3 = true;

    ultimoRebotePin3 = ahora;
  }
}