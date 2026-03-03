from flask import Flask, request, jsonify
import serial
import time

app = Flask(__name__)

arduino = serial.Serial('/dev/ttyACM0', 9600)
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
    return """
    <h2>Control Arduino</h2>
    <form action="/set" method="get">
        LED 9: <input type="number" name="l9" min="0" max="255"><br>
        LED 10: <input type="number" name="l10" min="0" max="255"><br>
        LED 11: <input type="number" name="l11" min="0" max="255"><br>
        LED 13 (0 o 1): <input type="number" name="l13" min="0" max="1"><br>
        <input type="submit" value="Enviar">
    </form>
    <br>
    <a href="/estado">Ver Estado</a>
    """

@app.route("/set")
def set_leds():
    l9 = request.args.get("l9")
    l10 = request.args.get("l10")
    l11 = request.args.get("l11")
    l13 = request.args.get("l13")

    if l9: enviar_comando(f"L9:{l9}")
    if l10: enviar_comando(f"L10:{l10}")
    if l11: enviar_comando(f"L11:{l11}")
    if l13: enviar_comando(f"L13:{l13}")

    return "Comandos enviados.<br><a href='/'>Volver</a>"

@app.route("/estado")
def estado():
    estado = obtener_estado()
    return f"<h3>Estado actual:</h3><p>{estado}</p><a href='/'>Volver</a>"

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)