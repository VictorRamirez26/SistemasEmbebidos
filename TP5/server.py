from flask import Flask, jsonify, render_template
import serial
import threading
import time
from datetime import datetime
from zoneinfo import ZoneInfo

app = Flask(__name__)

PUERTO_SERIE = "COM7"
BAUDRATE = 9600

datos = []

ultimo_timestamp = None

arduino = None


def conectar_arduino():

    global arduino

    while True:

        try:

            print("Intentando conectar Arduino...")

            arduino = serial.Serial(
                PUERTO_SERIE,
                BAUDRATE,
                timeout=1
            )

            time.sleep(2)

            print("Arduino conectado")

            return

        except Exception as e:

            print("No se pudo conectar:", e)

            time.sleep(3)


def leer_serial():

    global ultimo_timestamp
    global arduino

    while True:

        try:

            # reconectar si se desconectó
            if arduino is None or not arduino.is_open:
                conectar_arduino()

            linea = arduino.readline().decode().strip()

            if linea == "":
                continue

            valor = int(linea)

            horaActual = datetime.now(
                ZoneInfo("America/Argentina/Mendoza")
            )

            if ultimo_timestamp is not None:

                diferencia = (
                    horaActual - ultimo_timestamp
                ).total_seconds()

                # insertar discontinuidad
                if diferencia > 10:

                    datos.append({
                        "hora": horaActual.strftime("%H:%M:%S"),
                        "valor": None
                    })

            ultimo_timestamp = horaActual

            datos.append({
                "hora": horaActual.strftime("%H:%M:%S"),
                "valor": valor
            })

            print(datos[-1])

        except Exception as e:

            print("Error:", e)

            try:
                arduino.close()
            except:
                pass

            arduino = None

            time.sleep(2)


@app.route("/datos")
def obtener_datos():

    return jsonify(datos)


@app.route("/")
def index():

    return render_template("index.html")


thread_serial = threading.Thread(
    target=leer_serial,
    daemon=True
)

thread_serial.start()

app.run(
    host="0.0.0.0",
    port=5000,
    debug=False
)
