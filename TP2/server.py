import serial
import threading
from flask import Flask, jsonify , render_template

# --- Serial ---
arduino = serial.serial_for_url("rfc2217://localhost:4000", 9600) # simulador
last_value = 0

def read_serial():
    global last_value
    while True:
        try:
            line = arduino.readline().decode().strip()
            if line.isdigit():
                last_value = int(line)
        except:
            pass

# --- Flask ---
app = Flask(__name__)

@app.route("/luz")
def get_luz():
    return jsonify({"luz": last_value})

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

# --- Main ---
if __name__ == "__main__":
    thread = threading.Thread(target=read_serial)
    thread.daemon = True
    thread.start()

    app.run(debug=True)