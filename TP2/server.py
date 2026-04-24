import serial
import threading
from flask import Flask, jsonify , render_template
last_alarm = 0

# --- Serial ---
arduino = serial.serial_for_url("rfc2217://localhost:4000", 9600) # simulador
last_value = 0

def read_serial():
    global last_value
    global last_alarm

    while True:
        try:
            line = arduino.readline().decode().strip()

            if line.startswith("L:"):
                parts = line.split(",")

                for p in parts:
                    if p.startswith("L:"):
                        last_value = int(p.replace("L:", ""))

                    if p.startswith("A:"):
                        last_alarm = int(p.replace("A:", ""))

        except:
            pass

# --- Flask ---
app = Flask(__name__)


@app.route("/")
def index():
    return render_template("index.html")

@app.route("/start", methods=["POST"])
def start():
    arduino.write(b'1')  # enviar START
    return jsonify({"status": "ok", "action": "start"})

@app.route("/stop", methods=["POST"])
def stop():
    arduino.write(b'0')  # enviar STOP
    return jsonify({"status": "ok", "action": "stop"})

@app.route("/status")
def status():
    return jsonify({
        "luz": last_value,
        "alarm": last_alarm
    })

# --- Main ---
if __name__ == "__main__":
    thread = threading.Thread(target=read_serial)
    thread.daemon = True
    thread.start()

    app.run(debug=True)