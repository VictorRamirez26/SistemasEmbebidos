const ctx = document.getElementById("graficoLuz");

const grafico = new Chart(ctx, {
  type: "line",

  data: {
    labels: [],
    datasets: [
      {
        label: "Intensidad lumínica",
        data: [],
        borderWidth: 2,
        tension: 0.2,
      },
    ],
  },

  options: {
    responsive: true,

    animation: false,

    scales: {
      x: {
        title: {
          display: true,
          text: "Hora UTC-3",
        },

        ticks: {
          maxTicksLimit: 8,
        },
      },

      y: {
        title: {
          display: true,
          text: "Intensidad",
        },

        min: 0,
        max: 1023,
      },
    },
  },
});

async function actualizarGrafico() {
  try {
    const response = await fetch("/datos");

    const datos = await response.json();

    grafico.data.labels = datos.map((d) => d.hora);

    grafico.data.datasets[0].data = datos.map((d) => d.valor);

    grafico.update();
  } catch (error) {
    console.error(error);
  }
}

actualizarGrafico();

setInterval(actualizarGrafico, 5000);
