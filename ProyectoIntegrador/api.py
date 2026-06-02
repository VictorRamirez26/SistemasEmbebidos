from flask import Flask, jsonify, render_template, request
import serial
import threading
import time

app = Flask(__name__)


arduino = serial.Serial(
    port="COM7",
    baudrate=9600,
    timeout=1
)


#arduino = serial.serial_for_url("rfc2217://localhost:4000", 9600) # simulador

datos = {
    "level": 0,
    "status": "NORMAL",
    "min": 20,
    "max": 80,
    "alt": 100
}


def leer_serial():
    while True:
        try:
            if arduino.in_waiting:

                linea = arduino.readline().decode().strip()

                if not linea:
                    continue

                campos = linea.split(";")

                nuevo = {}

                for campo in campos:

                    if ":" not in campo:
                        continue

                    clave, valor = campo.split(":", 1)

                    nuevo[clave] = valor

                if "LEVEL" in nuevo:
                    datos["level"] = int(nuevo["LEVEL"])

                if "STATUS" in nuevo:
                    datos["status"] = nuevo["STATUS"]

                if "MIN" in nuevo:
                    datos["min"] = int(nuevo["MIN"])

                if "MAX" in nuevo:
                    datos["max"] = int(nuevo["MAX"])

                if "ALT" in nuevo:
                    datos["alt"] = int(nuevo["ALT"])

        except Exception as e:
            print("Error:", e)

        time.sleep(0.1)


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/datos")
def obtener_datos():
    return jsonify(datos)


@app.route("/configurar", methods=["POST"])
def configurar():

    try:

        minimo = int(request.form["min"])
        maximo = int(request.form["max"])
        altura = int(request.form["alt"])

        validarCampos(minimo, maximo, altura)

        trama = f"MIN:{minimo};MAX:{maximo};ALT:{altura}\n"

        arduino.write(trama.encode())
        arduino.flush()

        return jsonify({
            "ok": True,
            "mensaje": "Configuración enviada"
        })

    except ValueError:
        return jsonify({
            "ok": False,
            "error": "Todos los campos deben ser numéricos"
        }), 400

    except Exception as e:
        return jsonify({
            "ok": False,
            "error": str(e)
        }), 400

def validarCampos(minimo, maximo, altura):

    if altura <= 0:
        raise Exception(
            "La altura del tanque debe ser mayor a 0"
        )

    if altura > 200:
        raise Exception(
            "La altura del tanque no puede superar 200 cm"
        )

    if minimo < 0:
        raise Exception(
            "El nivel mínimo no puede ser negativo"
        )

    if maximo < 0:
        raise Exception(
            "El nivel máximo no puede ser negativo"
        )

    if minimo >= altura:
        raise Exception(
            "El nivel mínimo debe ser menor que la altura del tanque"
        )

    if maximo > altura:
        raise Exception(
            "El nivel máximo no puede superar la altura del tanque"
        )

    if minimo >= maximo:
        raise Exception(
            "El nivel mínimo debe ser menor que el nivel máximo"
        )

if __name__ == "__main__":

    hilo = threading.Thread(
        target=leer_serial,
        daemon=True
    )

    hilo.start()

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )