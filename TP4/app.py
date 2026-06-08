import threading
import time
import serial
from flask import Flask, jsonify, render_template
from datetime import datetime

app = Flask(__name__)

PUERTO_SERIE = "COM7"
BAUDRATE = 9600

estado = {
    "contador": 0,
    "alarma": 0,
    "persona_detectada": False,
    "timestamp": None,
    "conectado": False,
    "evento_nuevo": False
}

lock = threading.Lock()
ser = None
hilo_serial = None


def conexion_serial():
    global ser, estado

    while True:
        try:
            if ser is None or not ser.is_open:
                print(f"[SERIAL] Intentando conectar a {PUERTO_SERIE}...")
                ser = serial.Serial(PUERTO_SERIE, BAUDRATE, timeout=1)
                time.sleep(2)
                print(f"[SERIAL] Conectado a {PUERTO_SERIE}")
                with lock:
                    estado["conectado"] = True

            if ser.in_waiting:
                linea = ser.readline().decode("utf-8", errors="ignore").strip()

                if linea:
                    print(f"[SERIAL] Recibido: {linea}")
                    parsear_trama(linea)

            time.sleep(0.05)

        except serial.SerialException:
            print("[SERIAL] Error de conexión. Reconectando en 3s...")
            with lock:
                estado["conectado"] = False
            if ser:
                try:
                    ser.close()
                except Exception:
                    pass
                ser = None
            time.sleep(3)

        except Exception as e:
            print(f"[SERIAL] Error inesperado: {e}")
            time.sleep(1)


def parsear_trama(linea):
    global estado

    if linea == "EVENTO":
        with lock:
            estado["persona_detectada"] = True
            estado["evento_nuevo"] = True
            estado["timestamp"] = datetime.now().isoformat()
        return

    if linea.startswith("CONTADOR:") and ";ALARMA:" in linea:
        try:
            partes = linea.split(";")
            contador = int(partes[0].split(":")[1])
            alarma = int(partes[1].split(":")[1])
            with lock:
                estado["contador"] = contador
                estado["alarma"] = alarma
                estado["timestamp"] = datetime.now().isoformat()
        except (ValueError, IndexError) as e:
            print(f"[SERIAL] Error al parsear: {e}")


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/estado", methods=["GET"])
def obtener_estado():
    with lock:
        estado_copia = estado.copy()
        if estado_copia["evento_nuevo"]:
            estado["evento_nuevo"] = False
        estado_copia.pop("evento_nuevo", None)
    return jsonify(estado_copia)


@app.route("/api/reset", methods=["POST"])
def reset_contador():
    global ser
    try:
        if ser and ser.is_open:
            ser.write(b"RESET\n")
            print("[SERIAL] Comando RESET enviado")
            return jsonify({"ok": True, "mensaje": "Reset enviado"})
        else:
            return jsonify({"ok": False, "mensaje": "Arduino no conectado"}), 503
    except Exception as e:
        return jsonify({"ok": False, "mensaje": str(e)}), 500


@app.route("/api/reconectar", methods=["POST"])
def reconectar():
    global ser
    try:
        if ser and ser.is_open:
            ser.close()
        ser = None
        return jsonify({"ok": True, "mensaje": "Reconexión iniciada"})
    except Exception as e:
        return jsonify({"ok": False, "mensaje": str(e)}), 500


if __name__ == "__main__":
    hilo_serial = threading.Thread(target=conexion_serial, daemon=True)
    hilo_serial.start()
    print("[APP] Servidor iniciado en http://localhost:5000")
    app.run(host="0.0.0.0", port=5000, debug=False)
