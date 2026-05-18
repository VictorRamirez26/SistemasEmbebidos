from flask import Flask, render_template, jsonify
import serial
import ntplib
import time

app = Flask(__name__)

arduino = serial.serial_for_url("rfc2217://localhost:4000", 9600) 

time.sleep(2)


def obtener_hora_ntp():

    cliente = ntplib.NTPClient()

    respuesta = cliente.request('pool.ntp.org')

    return int(respuesta.tx_time)


def actualizar_hora_arduino():

    timestamp = obtener_hora_ntp()

    mensaje = f"TIME:{timestamp}\n"

    arduino.write(mensaje.encode())

    respuesta = arduino.readline().decode().strip()

    return timestamp

def obtener_hora_arduino():

    arduino.write(b"GETTIME\n")

    respuesta = arduino.readline().decode().strip()

    if respuesta.startswith("TIME:"):

        timestamp = int(respuesta.split(":")[1])

        return timestamp

    return None


@app.route('/')
def index():

    return render_template('index.html')


@app.route('/actualizar-hora')
def actualizar_hora():

    timestamp = actualizar_hora_arduino()

    fecha = time.ctime(timestamp)

    return jsonify({
        "timestamp": timestamp,
        "fecha": fecha
    })


@app.route('/hora')
def hora():

    timestamp = obtener_hora_arduino()

    fecha = time.ctime(timestamp)

    return jsonify({
        "timestamp": timestamp,
        "fecha": fecha
    })


if __name__ == '__main__':

    app.run(debug=True)