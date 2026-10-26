import {instrumentoActivo, colorDeInstrumento} from "./instrumentos.js";
import {
	CANALES,
	DURACION_INICIAL,
	DURACION_MIN,
	NOTA_MAX,
	SNAP_TICKS,
	nombreNota,
} from "./notas.js";
import {cambiarDuracionNota, eliminarNota, insertarNota, state, subscribe} from "../project.js";

const FILA = 18;
const PX_TICK = 1;
const BARRAS = 16;
const TICKS_BARRA = 192;

let canal = "Canal1";
let notaSel = null;
let musicaVista = null;
let arrastre = null;

function musicaNombre() {
	if (state.seleccion?.tipo !== "musica") {
		return null;
	}
	return state.seleccion.nombre;
}

function notasActuales() {
	const nombre = musicaNombre();
	if (!nombre) {
		return [];
	}
	return state.proyecto.Musica[nombre]?.Notas?.[canal] ?? [];
}

function ticksVisibles(notas) {
	let max = BARRAS * TICKS_BARRA;
	for (const nota of notas) {
		const fin = nota.Posicion + nota.Duracion + TICKS_BARRA;
		if (fin > max) {
			max = fin;
		}
	}
	return max;
}

function snap(ticks) {
	return Math.floor(ticks / SNAP_TICKS) * SNAP_TICKS;
}

function enganchar(ticks) {
	return Math.max(0, snap(ticks));
}

function pintarCanales() {
	const caja = document.getElementById("piano-channels");
	caja.replaceChildren();
	CANALES.forEach((nombre, i) => {
		const boton = document.createElement("button");
		boton.type = "button";
		boton.className = "piano-channel" + (nombre === canal ? " active" : "");
		boton.dataset.canal = nombre;
		boton.textContent = `Canal ${i + 1}`;
		boton.addEventListener("click", () => {
			canal = nombre;
			notaSel = null;
			pintar();
		});
		caja.append(boton);
	});
}

function pintarTeclas() {
	const caja = document.getElementById("piano-keys");
	caja.replaceChildren();
	for (let nota = NOTA_MAX; nota >= 0; nota--) {
		const fila = document.createElement("div");
		fila.className = "piano-key" + (esSostenido(nota) ? " sharp" : "");
		fila.textContent = nombreNota(nota);
		caja.append(fila);
	}
}

function esSostenido(nota) {
	return [1, 3, 6, 8, 10].includes(nota % 12);
}

function pintar() {
	const nombre = musicaNombre();
	const roll = document.getElementById("piano-roll");
	if (!roll) {
		return;
	}
	if (nombre !== musicaVista) {
		musicaVista = nombre;
		canal = "Canal1";
		notaSel = null;
		arrastre = null;
	}
	if (!nombre) {
		return;
	}

	pintarCanales();
	const notas = notasActuales();
	if (notaSel !== null && (notaSel < 0 || notaSel >= notas.length)) {
		notaSel = null;
	}

	const hint = document.getElementById("piano-hint");
	hint.textContent = instrumentoActivo() ? "" : "Agrega un instrumento para colocar notas";

	const ticks = ticksVisibles(notas);
	const alto = (NOTA_MAX + 1) * FILA;
	const ancho = ticks * PX_TICK;
	const grid = document.getElementById("piano-grid");
	grid.style.width = `${ancho}px`;
	grid.style.height = `${alto}px`;

	const keys = document.getElementById("piano-keys");
	keys.style.height = `${alto}px`;

	grid.replaceChildren();
	for (let nota = NOTA_MAX; nota >= 0; nota--) {
		const fila = document.createElement("div");
		fila.className = "piano-row" + (esSostenido(nota) ? " sharp" : "");
		fila.style.top = `${(NOTA_MAX - nota) * FILA}px`;
		fila.style.height = `${FILA}px`;
		grid.append(fila);
	}

	notas.forEach((nota, indice) => {
		const el = document.createElement("div");
		el.className = "piano-note" + (indice === notaSel ? " selected" : "");
		el.dataset.index = String(indice);
		el.dataset.nota = String(nota.Nota);
		el.dataset.pos = String(nota.Posicion);
		el.dataset.dur = String(nota.Duracion);
		el.style.left = `${nota.Posicion * PX_TICK}px`;
		el.style.top = `${(NOTA_MAX - nota.Nota) * FILA + 1}px`;
		el.style.width = `${Math.max(nota.Duracion * PX_TICK, 8)}px`;
		el.style.height = `${FILA - 2}px`;
		el.style.background = colorDeInstrumento(nota.Instrumento);
		const asa = document.createElement("div");
		asa.className = "piano-note-handle";
		el.append(asa);
		grid.append(el);
	});
}

function borrarSeleccion() {
	const nombre = musicaNombre();
	if (!nombre || notaSel === null) {
		return;
	}
	const indice = notaSel;
	notaSel = null;
	eliminarNota(nombre, canal, indice);
}

function alPointerDown(e) {
	if (state.seleccion?.tipo !== "musica") {
		return;
	}
	const asa = e.target.closest(".piano-note-handle");
	const notaEl = e.target.closest(".piano-note");
	if (asa && notaEl) {
		e.preventDefault();
		const indice = Number(notaEl.dataset.index);
		notaSel = indice;
		document.querySelectorAll(".piano-note.selected").forEach((el) => el.classList.remove("selected"));
		notaEl.classList.add("selected");
		const notas = notasActuales();
		arrastre = {
			indice,
			inicioX: e.clientX,
			inicioDur: notas[indice].Duracion,
			duracion: notas[indice].Duracion,
		};
		notaEl.classList.add("selected");
		notaEl.setPointerCapture(e.pointerId);
		return;
	}
	if (notaEl) {
		notaSel = Number(notaEl.dataset.index);
		pintar();
		return;
	}
	if (!e.target.closest("#piano-grid")) {
		return;
	}
	const instrumento = instrumentoActivo();
	if (!instrumento) {
		return;
	}
	const grid = document.getElementById("piano-grid");
	const rect = grid.getBoundingClientRect();
	const x = e.clientX - rect.left;
	const y = e.clientY - rect.top;
	const posicion = enganchar(x / PX_TICK);
	const fila = Math.floor(y / FILA);
	const nota = NOTA_MAX - fila;
	if (nota < 0 || nota > NOTA_MAX) {
		return;
	}
	const nombre = musicaNombre();
	notaSel = insertarNota(nombre, canal, {
		Instrumento: instrumento,
		Nota: nota,
		Posicion: posicion,
		Duracion: DURACION_INICIAL,
	});
	pintar();
}

function alPointerMove(e) {
	if (!arrastre) {
		return;
	}
	const delta = snap((e.clientX - arrastre.inicioX) / PX_TICK);
	let duracion = arrastre.inicioDur + delta;
	if (duracion < DURACION_MIN) {
		duracion = DURACION_MIN;
	}
	arrastre.duracion = duracion;
	const el = document.querySelector(`.piano-note[data-index="${arrastre.indice}"]`);
	if (el) {
		el.style.width = `${duracion * PX_TICK}px`;
		el.dataset.dur = String(duracion);
	}
}

function alPointerUp() {
	if (!arrastre) {
		return;
	}
	const {indice, inicioDur, duracion} = arrastre;
	const nombre = musicaNombre();
	arrastre = null;
	if (nombre && duracion !== inicioDur) {
		cambiarDuracionNota(nombre, canal, indice, duracion);
		return;
	}
	pintar();
}

export function initPiano() {
	pintarTeclas();
	document.getElementById("piano-grid").addEventListener("pointerdown", alPointerDown);
	window.addEventListener("pointermove", alPointerMove);
	window.addEventListener("pointerup", alPointerUp);
	document.getElementById("piano-delete").addEventListener("click", borrarSeleccion);
	window.addEventListener("keydown", (e) => {
		if (e.key !== "Delete" && e.key !== "Backspace") {
			return;
		}
		if (state.seleccion?.tipo !== "musica" || notaSel === null) {
			return;
		}
		const tag = document.activeElement?.tagName;
		if (tag === "INPUT" || tag === "SELECT" || tag === "TEXTAREA") {
			return;
		}
		e.preventDefault();
		borrarSeleccion();
	});
	document.addEventListener("sb-instrumento", () => pintar());
	subscribe(pintar);
	pintar();
}
