from flask import Flask, request, jsonify, render_template
import serial
import time

app = Flask(__name__)

#arduino = serial.serial_for_url("rfc2217://localhost:4000", 9600) # simulador
arduino = serial.Serial('COM7', 9600)
time.sleep(2)

def enviar_comando(cmd):
    arduino.write((cmd + "\n").encode())
    time.sleep(0.1)

def obtener_estado():
    arduino.write(b"GET\n")
    respuesta = arduino.readline().decode().strip()
    return respuesta

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/set", methods=["POST"])
def set_leds():
    data = request.json

    if "l9" in data: enviar_comando(f"L9:{data['l9']}")
    if "l10" in data: enviar_comando(f"L10:{data['l10']}")
    if "l11" in data: enviar_comando(f"L11:{data['l11']}")
    if "l13" in data: enviar_comando(f"L13:{data['l13']}")

    return jsonify({"status": "ok"})

@app.route("/estado")
def estado():
    return jsonify({"estado": obtener_estado()})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)