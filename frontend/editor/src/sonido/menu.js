import {bytesABase64, bytesDeBase64, validarBrr} from "./brr.js";
import {LIBRERIA} from "./libreria.js";
import {incorporarSonido, state, subscribe} from "../project.js";

function cerrarMenu() {
	const menu = document.getElementById("sound-menu");
	const lista = document.getElementById("sound-library");
	if (menu) {
		menu.hidden = true;
	}
	if (lista) {
		lista.hidden = true;
	}
}

function abrirMenu() {
	const boton = document.getElementById("add-sonido");
	const menu = document.getElementById("sound-menu");
	const rect = boton.getBoundingClientRect();
	menu.hidden = false;
	menu.style.left = `${rect.left}px`;
	menu.style.top = `${rect.bottom + 4}px`;
}

function tomarSonido(nombre, bytes) {
	try {
		const {datos, puntoLoop} = validarBrr(bytes);
		incorporarSonido(nombre, bytesABase64(datos), puntoLoop);
	} catch (e) {
		window.alert(e instanceof Error ? e.message : String(e));
	}
}

function pintarDetalle() {
	const panel = document.getElementById("sound-detail");
	if (!panel) {
		return;
	}
	const sel = state.seleccion;
	if (!sel || sel.tipo !== "sonido") {
		panel.replaceChildren();
		return;
	}
	const sonido = state.proyecto.Sonidos[sel.nombre];
	panel.replaceChildren();
	if (!sonido) {
		return;
	}
	const bytes = bytesDeBase64(sonido.Datos);
	const bloques = bytes > 0 ? bytes / 9 : 0;
	const titulo = document.createElement("div");
	titulo.className = "sound-name";
	titulo.textContent = sel.nombre;
	const sample = document.createElement("div");
	sample.textContent = `Sample: ${bytes} bytes (${bloques} bloques)`;
	const loop = document.createElement("div");
	loop.textContent = `Punto de loop: ${sonido.PuntoLoop}`;
	panel.append(titulo, sample, loop);
}

export function initSonidos() {
	const biblioteca = document.getElementById("sound-library");
	for (const item of LIBRERIA) {
		const boton = document.createElement("button");
		boton.type = "button";
		boton.className = "menu-item";
		boton.dataset.lib = item.nombre;
		boton.textContent = item.nombre;
		boton.addEventListener("click", () => {
			cerrarMenu();
			tomarSonido(item.nombre, item.bytes);
		});
		biblioteca.append(boton);
	}

	document.getElementById("add-sonido").addEventListener("click", (e) => {
		e.stopPropagation();
		const menu = document.getElementById("sound-menu");
		if (menu.hidden) {
			abrirMenu();
		} else {
			cerrarMenu();
		}
	});

	document.getElementById("sound-menu-library").addEventListener("click", () => {
		const lista = document.getElementById("sound-library");
		lista.hidden = !lista.hidden;
	});

	document.getElementById("sound-menu-upload").addEventListener("click", () => {
		cerrarMenu();
		document.getElementById("sound-file").click();
	});

	document.getElementById("sound-file").addEventListener("change", async (e) => {
		const input = e.target;
		const archivo = input.files && input.files[0];
		input.value = "";
		if (!archivo) {
			return;
		}
		const nombre = archivo.name.replace(/\.brr$/i, "");
		const bytes = new Uint8Array(await archivo.arrayBuffer());
		tomarSonido(nombre, bytes);
	});

	document.addEventListener("click", (e) => {
		if (e.target.closest("#sound-menu") || e.target.closest("#add-sonido")) {
			return;
		}
		cerrarMenu();
	});

	document.addEventListener("keydown", (e) => {
		if (e.key === "Escape") {
			cerrarMenu();
		}
	});

	subscribe(pintarDetalle);
	pintarDetalle();
}
