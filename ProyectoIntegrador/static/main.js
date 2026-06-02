// Altura del tanque actual (se actualiza desde /datos)
let ALTURA = 100;

// ── Polling ───────────────────────────────────────────────────────────────────
async function fetchDatos() {
  try {
    const r = await fetch("/datos");
    if (!r.ok) return;
    render(await r.json());
  } catch (_) {}
}

// ── Render ────────────────────────────────────────────────────────────────────
function render(d) {
  ALTURA = d.alt || 100;

  // badge conexión — si llegaron datos, está conectado
  const badge = document.getElementById("badge");
  badge.textContent = "CONECTADO";
  badge.classList.add("on");

  // tanque
  const pct = Math.min(100, Math.max(0, (d.level / ALTURA) * 100));
  document.getElementById("tankFill").style.height = pct + "%";
  document.getElementById("tankVal").textContent = d.level + " cm";
  document.getElementById("lineMin").style.bottom =
    (d.min / ALTURA) * 100 + "%";
  document.getElementById("lineMax").style.bottom =
    (d.max / ALTURA) * 100 + "%";

  // métricas
  document.getElementById("mLevel").textContent = d.level;
  document.getElementById("mAlt").textContent = d.alt;
  document.getElementById("mMin").textContent = d.min;
  document.getElementById("mMax").textContent = d.max;

  // status pill
  const pill = document.getElementById("statusPill");
  pill.textContent = d.status;
  pill.className = "status-pill " + d.status;

  // válvulas
  setValve("vEntrada", "active-entrada", d.status === "LOW");
  setValve("vSalida", "active-salida", d.status === "HIGH");
  setValve(
    "vAlarma",
    "active-alarma",
    d.status === "LOW" || d.status === "HIGH",
  );

  // sync inputs
  document.getElementById("inputMin").placeholder = d.min;
  document.getElementById("inputMax").placeholder = d.max;
  document.getElementById("inputAlt").placeholder = d.alt;

  drawChart(d.level, d.min, d.max);
}

function setValve(id, cls, active) {
  const el = document.getElementById(id);
  el.className = "valve" + (active ? " " + cls : "");
}

// ── Configurar ────────────────────────────────────────────────────────────────
async function aplicarConfig() {
  const min = document.getElementById("inputMin").value;
  const max = document.getElementById("inputMax").value;
  const alt = document.getElementById("inputAlt").value;

  if (!min || !max || !alt) {
    showToast("⚠ Completá todos los campos", true);
    return;
  }

  const form = new FormData();

  form.append("min", min);
  form.append("max", max);
  form.append("alt", alt);

  try {
    const response = await fetch("/configurar", {
      method: "POST",
      body: form,
    });

    const data = await response.json();

    if (!response.ok || !data.ok) {
      showToast(`⚠ ${data.error}`, true);
      return;
    }

    showToast(`✓ ${data.mensaje}`, false);
  } catch (error) {
    showToast("⚠ Error de conexión con el servidor", true);
  }
}

// ── Gráfica ───────────────────────────────────────────────────────────────────
const history = [];

function drawChart(currentLevel, min, max) {
  // acumular historial (máx 60 puntos)
  history.push(currentLevel);
  if (history.length > 60) history.shift();

  const canvas = document.getElementById("chart");
  const ctx = canvas.getContext("2d");
  const dpr = window.devicePixelRatio || 1;
  canvas.width = canvas.offsetWidth * dpr;
  canvas.height = 110 * dpr;
  ctx.scale(dpr, dpr);

  const W = canvas.offsetWidth,
    H = 110;
  const pad = { t: 8, r: 8, b: 22, l: 36 };

  ctx.clearRect(0, 0, W, H);

  // grid + etiquetas Y (en cm)
  ctx.font = "9px 'Share Tech Mono', monospace";
  ctx.textAlign = "right";
  [0, 0.25, 0.5, 0.75, 1].forEach((frac) => {
    const v = Math.round(frac * ALTURA);
    const y = pad.t + (H - pad.t - pad.b) * (1 - frac);
    ctx.fillStyle = "rgba(255,255,255,.04)";
    ctx.fillRect(pad.l, y, W - pad.l - pad.r, 1);
    ctx.fillStyle = "#484f58";
    ctx.fillText(v, pad.l - 4, y + 3);
  });

  // líneas punteadas min/max
  const toY = (v) =>
    pad.t +
    (H - pad.t - pad.b) * (1 - Math.min(ALTURA, Math.max(0, v)) / ALTURA);

  ctx.setLineDash([4, 4]);
  ctx.lineWidth = 1;
  ctx.strokeStyle = "rgba(227,179,65,.5)";
  ctx.beginPath();
  ctx.moveTo(pad.l, toY(min));
  ctx.lineTo(W - pad.r, toY(min));
  ctx.stroke();
  ctx.strokeStyle = "rgba(248,81,73,.5)";
  ctx.beginPath();
  ctx.moveTo(pad.l, toY(max));
  ctx.lineTo(W - pad.r, toY(max));
  ctx.stroke();
  ctx.setLineDash([]);

  if (history.length < 2) return;

  const xStep = (W - pad.l - pad.r) / (history.length - 1);
  const toX = (i) => pad.l + i * xStep;

  // área
  ctx.beginPath();
  ctx.moveTo(toX(0), toY(history[0]));
  history.forEach((v, i) => ctx.lineTo(toX(i), toY(v)));
  ctx.lineTo(toX(history.length - 1), H - pad.b);
  ctx.lineTo(toX(0), H - pad.b);
  ctx.closePath();
  const g = ctx.createLinearGradient(0, pad.t, 0, H);
  g.addColorStop(0, "rgba(88,166,255,.3)");
  g.addColorStop(1, "rgba(88,166,255,.02)");
  ctx.fillStyle = g;
  ctx.fill();

  // línea
  ctx.beginPath();
  ctx.moveTo(toX(0), toY(history[0]));
  history.forEach((v, i) => ctx.lineTo(toX(i), toY(v)));
  ctx.strokeStyle = "rgba(88,166,255,.9)";
  ctx.lineWidth = 1.5;
  ctx.lineJoin = "round";
  ctx.stroke();
}

// ── Toast ─────────────────────────────────────────────────────────────────────
function showToast(msg, err = false) {
  const t = document.getElementById("toast");
  t.textContent = msg;
  t.style.background = err ? "#f85149" : "#e6edf3";
  t.style.color = "#0d1117";
  t.classList.add("show");
  setTimeout(() => t.classList.remove("show"), 2500);
}

// ── Arrancar ──────────────────────────────────────────────────────────────────
setInterval(fetchDatos, 1000);
fetchDatos();
