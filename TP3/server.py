from flask import Flask, render_template, jsonify
import serial
import ntplib
import time

app = Flask(__name__)

# arduino = serial.serial_for_url("rfc2217://localhost:4000", 9600)
# arduino = serial.Serial('COM7', 9600)
arduino = serial.Serial('/dev/ttyUSB1', 9600)
time.sleep(3)


def obtener_hora_ntp():

    cliente = ntplib.NTPClient()

    respuesta = cliente.request('pool.ntp.org')

    return int(respuesta.tx_time)


def actualizar_hora_arduino():

    timestamp = obtener_hora_ntp()

    mensaje = f"TIME:{timestamp}\n"

    arduino.write(mensaje.encode())

    respuesta = arduino.readline().decode().strip()

    print(respuesta)

    return timestamp


def obtener_hora_arduino():

    arduino.write(b"GETTIME\n")

    respuesta = arduino.readline().decode().strip()

    if respuesta.startswith("TIME:"):

        timestamp = int(respuesta.split(":")[1])

        return timestamp

    return None


def obtener_eventos():

    arduino.write(b"GETEVENT\n")

    eventos = []

    while True:

        linea = arduino.readline().decode().strip()

        print(linea)

        if linea == "END_EVENTS":
            break

        if linea.startswith("EVENT:"):

            datos = linea[6:]

            timestamp_str, evento_str = datos.split(",")

            timestamp = int(timestamp_str)

            evento = int(evento_str)

            fecha = time.ctime(timestamp)

            eventos.append({
                "timestamp": timestamp,
                "fecha": fecha,
                "evento": evento
            })

    return eventos


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


@app.route('/eventos')
def eventos():

    lista_eventos = obtener_eventos()

    return jsonify(lista_eventos)

@app.route('/borrar-eventos', methods=['POST'])
def borrar_eventos():
    arduino.write(b"CLEAREEPROM\n")  
    respuesta = arduino.readline().decode().strip()  
    return jsonify({"respuesta": respuesta})

if __name__ == '__main__':

    app.run(debug=False)
