function enviar() {
    const data = {
        l9: document.getElementById("l9").value,
        l10: document.getElementById("l10").value,
        l11: document.getElementById("l11").value,
        l13: document.getElementById("l13").value
    };

    fetch("/set", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(data)
    })
    .then(response => response.json())
    .then(data => alert("Comandos enviados"));
}

function actualizarEstado() {
    fetch("/estado")
    .then(response => response.json())
    .then(data => {
        document.getElementById("estado").innerText = data.estado;
    });
}