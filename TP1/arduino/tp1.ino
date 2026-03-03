const int LED_9 = 9;
const int LED_10 = 10;
const int LED_11 = 11;
const int LED_13 = 13;
const int LDR = A3;

int brillo9 = 0;
int brillo10 = 0;
int brillo11 = 0;
int brillo13 = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_9, OUTPUT);
  pinMode(LED_10, OUTPUT);
  pinMode(LED_11, OUTPUT);
  pinMode(LED_13, OUTPUT);
}

void loop() {

  // --- RECIBIR COMANDOS ---
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    procesarComando(comando);
  }
}

void procesarComando(String cmd) {

  if (cmd == "GET") {
    enviarEstado();
  }

  else if (cmd.startsWith("L9:")) {
    brillo9 = cmd.substring(3).toInt();
    analogWrite(LED_9, brillo9);
  }

  else if (cmd.startsWith("L10:")) {
    brillo10 = cmd.substring(4).toInt();
    analogWrite(LED_10, brillo10);
  }

  else if (cmd.startsWith("L11:")) {
    brillo11 = cmd.substring(4).toInt();
    analogWrite(LED_11, brillo11);
  }

  else if (cmd.startsWith("L13:")) {
    brillo13 = cmd.substring(4).toInt();
    digitalWrite(LED_13, brillo13 ? HIGH : LOW);
  }
}

void enviarEstado() {

  int valorLuz = analogRead(LDR);

  Serial.print("L9:"); Serial.print(brillo9);
  Serial.print(";L10:"); Serial.print(brillo10);
  Serial.print(";L11:"); Serial.print(brillo11);
  Serial.print(";L13:"); Serial.print(brillo13);
  Serial.print(";LDR:"); Serial.println(valorLuz);
}
