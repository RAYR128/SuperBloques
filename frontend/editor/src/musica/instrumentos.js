import {
	ATAQUE,
	DECAY,
	etiquetaAdsR,
	NOTAS_FRECUENCIA,
	RELEASE,
	SUSTAIN,
	colorDeInstrumento,
	notaDeFrecuencia,
} from "./notas.js";
import {
	agregarInstrumento,
	eliminarInstrumento,
	state,
	subscribe,
} from "../project.js";

let activo = null;

export function instrumentoActivo() {
	if (state.seleccion?.tipo !== "musica") {
		return null;
	}
	const musica = state.proyecto.Musica[state.seleccion.nombre];
	if (!musica || !musica.Instrumentos[activo]) {
		return null;
	}
	return activo;
}

export {colorDeInstrumento};

function musicaActual() {
	if (state.seleccion?.tipo !== "musica") {
		return null;
	}
	return state.proyecto.Musica[state.seleccion.nombre] ?? null;
}

function llenarSelect(select, entradas) {
	select.replaceChildren();
	for (const [valor, texto] of entradas) {
		const opcion = document.createElement("option");
		opcion.value = String(valor);
		opcion.textContent = texto;
		select.append(opcion);
	}
}

function llenarAdsR(select, textos) {
	llenarSelect(
		select,
		textos.map((texto, i) => [i, etiquetaAdsR(i, texto)]),
	);
}

function instrumentoSeleccionado() {
	const musica = musicaActual();
	if (!musica || !activo) {
		return null;
	}
	return musica.Instrumentos[activo] ?? null;
}

function pintarLista() {
	const lista = document.getElementById("instrument-list");
	const musica = musicaActual();
	lista.replaceChildren();
	if (!musica) {
		return;
	}
	const nombres = Object.keys(musica.Instrumentos);
	if (!nombres.length) {
		const vacio = document.createElement("div");
		vacio.className = "instrument-empty";
		vacio.textContent = "Sin instrumentos";
		lista.append(vacio);
		return;
	}
	for (const nombre of nombres) {
		const fila = document.createElement("button");
		fila.type = "button";
		fila.className = "instrument-item" + (nombre === activo ? " active" : "");
		const swatch = document.createElement("span");
		swatch.className = "instrument-swatch";
		swatch.style.background = colorDeInstrumento(nombre);
		const label = document.createElement("span");
		label.textContent = nombre;
		fila.append(swatch, label);
		fila.addEventListener("click", () => {
			activo = nombre;
			notifyDesdeLista();
		});
		lista.append(fila);
	}
}

function notifyDesdeLista() {
	pintarLista();
	pintarFormulario();
	document.dispatchEvent(new CustomEvent("sb-instrumento"));
}

function pintarFormulario() {
	const form = document.getElementById("instrument-form");
	const inst = instrumentoSeleccionado();
	if (!inst) {
		form.hidden = true;
		return;
	}
	form.hidden = false;

	const sonido = document.getElementById("inst-sonido");
	llenarSelect(
		sonido,
		Object.keys(state.proyecto.Sonidos).map((nombre) => [nombre, nombre]),
	);
	sonido.value = inst.Sonido;

	const nota = document.getElementById("inst-nota");
	const conocida = notaDeFrecuencia(inst.Frecuencia);
	nota.value = conocida ? String(conocida.frecuencia) : "";
	document.getElementById("inst-frecuencia").value = String(inst.Frecuencia);

	const esGain = document.getElementById("inst-esgain");
	esGain.checked = inst.EsGain;
	document.getElementById("inst-adsr").hidden = inst.EsGain;
	document.getElementById("inst-gain-wrap").hidden = !inst.EsGain;

	document.getElementById("inst-a").value = String(inst.A);
	document.getElementById("inst-d").value = String(inst.D);
	document.getElementById("inst-s").value = String(inst.S);
	document.getElementById("inst-r").value = String(inst.R);
	document.getElementById("inst-gain").value = String(inst.Gain);
}

function guardarCampo(fn) {
	const inst = instrumentoSeleccionado();
	if (!inst) {
		return;
	}
	fn(inst);
	pintarFormulario();
	document.dispatchEvent(new CustomEvent("sb-instrumento"));
}

function enteroInput(input, min, max) {
	const n = Number(input.value);
	if (!Number.isInteger(n)) {
		return null;
	}
	if (n < min || n > max) {
		return null;
	}
	return n;
}

export function initInstrumentos() {
	llenarAdsR(document.getElementById("inst-a"), ATAQUE);
	llenarAdsR(document.getElementById("inst-d"), DECAY);
	llenarAdsR(document.getElementById("inst-s"), SUSTAIN);
	llenarAdsR(document.getElementById("inst-r"), RELEASE);

	const nota = document.getElementById("inst-nota");
	llenarSelect(nota, [
		["", "personalizada"],
		...NOTAS_FRECUENCIA.map((n) => [n.frecuencia, `${n.nombre}  ${n.frecuencia}`]),
	]);

	document.getElementById("add-instrumento").addEventListener("click", () => {
		if (state.seleccion?.tipo !== "musica") {
			return;
		}
		const nombre = agregarInstrumento(state.seleccion.nombre);
		if (nombre) {
			activo = nombre;
			pintarLista();
			pintarFormulario();
			document.dispatchEvent(new CustomEvent("sb-instrumento"));
		}
	});

	document.getElementById("remove-instrumento").addEventListener("click", () => {
		if (state.seleccion?.tipo !== "musica" || !activo) {
			return;
		}
		eliminarInstrumento(state.seleccion.nombre, activo);
	});

	document.getElementById("inst-sonido").addEventListener("change", (e) => {
		guardarCampo((inst) => {
			inst.Sonido = e.target.value;
		});
	});

	document.getElementById("inst-nota").addEventListener("change", (e) => {
		if (e.target.value === "") {
			return;
		}
		const frecuencia = Number(e.target.value);
		guardarCampo((inst) => {
			inst.Frecuencia = frecuencia;
		});
	});

	document.getElementById("inst-frecuencia").addEventListener("change", (e) => {
		const frecuencia = enteroInput(e.target, 0, 65535);
		if (frecuencia === null) {
			pintarFormulario();
			return;
		}
		guardarCampo((inst) => {
			inst.Frecuencia = frecuencia;
		});
	});

	document.getElementById("inst-esgain").addEventListener("change", (e) => {
		guardarCampo((inst) => {
			inst.EsGain = e.target.checked;
		});
	});

	for (const [id, clave, max] of [
		["inst-a", "A", 15],
		["inst-d", "D", 7],
		["inst-s", "S", 7],
		["inst-r", "R", 31],
	]) {
		document.getElementById(id).addEventListener("change", (e) => {
			const valor = enteroInput(e.target, 0, max);
			if (valor === null) {
				return;
			}
			guardarCampo((inst) => {
				inst[clave] = valor;
			});
		});
	}

	document.getElementById("inst-gain").addEventListener("change", (e) => {
		const valor = enteroInput(e.target, 0, 127);
		if (valor === null) {
			pintarFormulario();
			return;
		}
		guardarCampo((inst) => {
			inst.Gain = valor;
		});
	});

	subscribe(() => {
		const musica = musicaActual();
		const nombres = musica ? Object.keys(musica.Instrumentos) : [];
		if (!nombres.includes(activo)) {
			activo = nombres[0] ?? null;
		}
		pintarLista();
		pintarFormulario();
	});
}
