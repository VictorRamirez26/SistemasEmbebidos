const fecha = document.getElementById("fecha");

const btnActualizar = document.getElementById("btnActualizar");

const btnEventos = document.getElementById("btnEventos");

const btnBorrar = document.getElementById("btnBorrar");

const eventosDiv = document.getElementById("eventos");


btnBorrar.addEventListener("click", async () => {
  const confirmar = confirm(
    "¿Seguro que deseas borrar todos los eventos guardados?"
  );

  if (!confirmar) return;

  const response = await fetch("/borrar-eventos", {
    method: "POST",
  });

  const data = await response.json();

  eventosDiv.innerHTML = "<p>Memoria EEPROM vaciada</p>";
});

btnActualizar.addEventListener("click", async () => {
  const response = await fetch("/actualizar-hora");

  const data = await response.json();

  fecha.innerText = data.fecha;
});


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


async function obtenerHora() {
  const response = await fetch("/hora");

  const data = await response.json();

  fecha.innerText = data.fecha;
}

// actualizar cada segundo
setInterval(obtenerHora, 2000);

// cargar inicialmente
obtenerHora();
