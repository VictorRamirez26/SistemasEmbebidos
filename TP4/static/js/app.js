let contadorAnterior = 0;

function actualizarUI(datos) {
    document.getElementById("contador").textContent = datos.contador;

    const statusDot = document.getElementById("status-dot");
    const statusText = document.getElementById("status-text");
    if (datos.conectado) {
        statusDot.className = "dot conectado";
        statusText.textContent = "Conectado";
    } else {
        statusDot.className = "dot desconectado";
        statusText.textContent = "Desconectado";
    }

    const alarmaIndicator = document.getElementById("alarma-indicator");
    const alarmaText = document.getElementById("alarma-text");
    const alarmaCard = document.querySelector(".alarma-card");
    if (datos.alarma) {
        alarmaIndicator.className = "dot alarma-on";
        alarmaText.textContent = "ALERTA ACTIVA";
        alarmaCard.classList.add("alarma-activa");
    } else {
        alarmaIndicator.className = "dot alarma-off";
        alarmaText.textContent = "Inactiva";
        alarmaCard.classList.remove("alarma-activa");
    }

    const deteccionIndicator = document.getElementById("deteccion-indicator");
    const deteccionText = document.getElementById("deteccion-text");
    if (datos.persona_detectada) {
        deteccionIndicator.className = "dot deteccion-on";
        deteccionText.textContent = "Persona detectada";
    } else {
        deteccionIndicator.className = "dot deteccion-off";
        deteccionText.textContent = "Sin presencia";
    }

    if (datos.timestamp) {
        const fecha = new Date(datos.timestamp);
        const opciones = {
            hour: "2-digit",
            minute: "2-digit",
            second: "2-digit",
            hour12: false
        };
        document.getElementById("timestamp").textContent = fecha.toLocaleTimeString("es-AR", opciones);
    }

    if (datos.contador > contadorAnterior) {
        mostrarEvento();
    }
    contadorAnterior = datos.contador;
}

function mostrarEvento() {
    const flash = document.getElementById("event-flash");
    flash.classList.remove("show");
    void flash.offsetWidth;
    flash.classList.add("show");
    setTimeout(() => {
        flash.classList.remove("show");
    }, 2000);
}

async function fetchEstado() {
    try {
        const response = await fetch("/api/estado");
        const datos = await response.json();
        actualizarUI(datos);
    } catch (error) {
        console.error("Error al obtener estado:", error);
        document.getElementById("status-dot").className = "dot desconectado";
        document.getElementById("status-text").textContent = "Error de conexión";
    }
}

async function resetContador() {
    const btn = document.getElementById("btn-reset");
    btn.disabled = true;
    btn.textContent = "Reseteando...";

    try {
        const response = await fetch("/api/reset", { method: "POST" });
        const result = await response.json();
        if (result.ok) {
            mostrarEvento();
        } else {
            alert("Error: " + result.mensaje);
        }
    } catch (error) {
        alert("Error al enviar reset: " + error.message);
    } finally {
        btn.disabled = false;
        btn.textContent = "Resetear Contador";
    }
}

async function reconectar() {
    const btn = document.getElementById("btn-reconectar");
    btn.disabled = true;
    btn.textContent = "Reconectando...";

    try {
        await fetch("/api/reconectar", { method: "POST" });
    } catch (error) {
        console.error("Error al reconectar:", error);
    } finally {
        setTimeout(() => {
            btn.disabled = false;
            btn.textContent = "Reconectar";
        }, 3000);
    }
}

document.addEventListener("DOMContentLoaded", () => {
    fetchEstado();
    setInterval(fetchEstado, 500);
});
