const fecha = document.getElementById("fecha");

const btnActualizar = document.getElementById("btnActualizar");

const btnEventos = document.getElementById("btnEventos");

const eventosDiv = document.getElementById("eventos");

// =======================================
// ACTUALIZAR DESDE NTP
// =======================================

btnActualizar.addEventListener("click", async () => {
  const response = await fetch("/actualizar-hora");

  const data = await response.json();

  fecha.innerText = data.fecha;
});

// =======================================
// OBTENER EVENTOS EEPROM
// =======================================

btnEventos.addEventListener("click", async () => {
  const response = await fetch("/eventos");

  const eventos = await response.json();

  eventosDiv.innerHTML = "";

  if (eventos.length === 0) {
    eventosDiv.innerHTML = "<p>No hay eventos guardados</p>";

    return;
  }

  eventos.forEach((evento) => {
    eventosDiv.innerHTML += `
            <div class="evento">
                <p><b>Evento:</b> Pin ${evento.evento}</p>
                <p><b>Fecha:</b> ${evento.fecha}</p>
                <hr>
            </div>
        `;
  });
});

// =======================================
// OBTENER HORA ACTUAL AUTOMATICAMENTE
// =======================================

async function obtenerHora() {
  const response = await fetch("/hora");

  const data = await response.json();

  fecha.innerText = data.fecha;
}

// actualizar cada segundo
setInterval(obtenerHora, 1000);

// cargar inicialmente
obtenerHora();
