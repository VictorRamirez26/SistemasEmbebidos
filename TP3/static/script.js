const fecha = document.getElementById("fecha");

const btnActualizar = document.getElementById("btnActualizar");


// =======================================
// ACTUALIZAR DESDE NTP
// =======================================

btnActualizar.addEventListener("click", async () => {

    const response = await fetch("/actualizar-hora");

    const data = await response.json();

    fecha.innerText = data.fecha;
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