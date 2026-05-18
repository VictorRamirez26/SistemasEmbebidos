#include <Arduino.h>
#include <EEPROM.h>

unsigned long unixTime = 0;

unsigned long ultimoIncremento = 0;

String buffer = "";

volatile bool eventoPin2 = false;
volatile bool eventoPin3 = false;

// byte 0 y 1:
// guardan la próxima dirección libre

int proximaDireccion = 2;

void procesarMensaje(String msg);

void guardarEvento(byte evento);

void interrupcionPin2();

void interrupcionPin3();

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

  // Recuperar puntero eeprom

  EEPROM.get(0, proximaDireccion);

  // EEPROM vacía
  if (proximaDireccion == 65535) {

    proximaDireccion = 2;
  }

  Serial.println("ARDUINO_LISTO");
}

void loop() {

  if (millis() - ultimoIncremento >= 1000) {

    unixTime++;

    ultimoIncremento = millis();
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


void procesarMensaje(String msg) {

  msg.trim();

  if (msg.startsWith("TIME:")) {

    String valor = msg.substring(5);

    unixTime = valor.toInt();

    Serial.println("OK_TIME_UPDATED");
  } else if (msg == "GETTIME") {

    Serial.print("TIME:");

    Serial.println(unixTime);
  } else {

    Serial.println("ERROR_UNKNOWN_COMMAND");
  }
}

void guardarEvento(byte evento) {

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

  EEPROM.put(0, proximaDireccion);


  Serial.print("EVENTO_GUARDADO en EEPROM: ");
  Serial.println(proximaDireccion);
}

void interrupcionPin2() {

  eventoPin2 = true;
}

void interrupcionPin3() {

  eventoPin3 = true;
}