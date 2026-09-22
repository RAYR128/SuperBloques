import {flushWorkspace} from "./blocks/workspace.js";
import {NOMBRE_ARCHIVO_DEFECTO, state} from "./project.js";

function informar(html) {
	const el = document.querySelector(".debug-body");
	if (el) {
		el.innerHTML = html;
	}
}

function escapar(texto) {
	return String(texto)
		.replace(/&/g, "&amp;")
		.replace(/</g, "&lt;")
		.replace(/>/g, "&gt;");
}

let moduloPromise = null;

function cargarModulo() {
	if (!moduloPromise) {
		const glue = new URL(`${import.meta.env.BASE_URL}compilador/superbloques.js`, location.origin).href;
		moduloPromise = import(/* @vite-ignore */ glue)
			.then((mod) => {
				const factory = mod.default ?? mod;
				return factory({
					locateFile: (path) => new URL(path, glue).href,
				});
			})
			.catch((e) => {
				moduloPromise = null;
				throw e;
			});
	}
	return moduloPromise;
}

function nombreRom() {
	const nombre = state.nombreArchivo || NOMBRE_ARCHIVO_DEFECTO;
	return `${nombre.replace(/\.json$/i, "")}.sfc`;
}

function descargar(bytes, nombre) {
	const blob = new Blob([bytes], {type: "application/octet-stream"});
	const url = URL.createObjectURL(blob);
	const a = document.createElement("a");
	a.href = url;
	a.download = nombre;
	a.click();
	URL.revokeObjectURL(url);
}

let compilacion = Promise.resolve();

async function compilarAhora() {
	flushWorkspace();
	informar('<div class="dim">Cargando compilador...</div>');
	let compilador;
	try {
		compilador = await cargarModulo();
	} catch {
		informar('<div class="warn">[error] no esta el compilador WASM</div>');
		return null;
	}

	informar('<div class="dim">Compilando...</div>');
	await new Promise((resolver) => setTimeout(resolver, 0));

	const json = JSON.stringify(state.proyecto);
	const bytes = new TextEncoder().encode(json);
	const ptr = compilador._malloc(bytes.length + 1);
	if (!ptr) {
		informar('<div class="warn">[error] sin memoria para el proyecto</div>');
		return null;
	}
	try {
		compilador.HEAPU8.set(bytes, ptr);
		compilador.HEAPU8[ptr + bytes.length] = 0;
		const rc = compilador._sb_compilar(ptr, bytes.length);
		if (rc !== 0) {
			const msg = compilador.UTF8ToString(compilador._sb_ultimo_error());
			informar(`<div class="warn">[error] ${escapar(msg)}</div>`);
			return null;
		}
		const romPtr = compilador._sb_rom();
		const n = compilador._sb_rom_tamano();
		return compilador.HEAPU8.slice(romPtr, romPtr + n);
	} finally {
		compilador._free(ptr);
	}
}

export function compilarRom() {
	const siguiente = compilacion.then(() => compilarAhora(), () => compilarAhora());
	compilacion = siguiente.then(() => undefined, () => undefined);
	return siguiente;
}

export async function crearYDescargarRom(boton) {
	if (boton) {
		boton.disabled = true;
	}
	try {
		const rom = await compilarRom();
		if (!rom) {
			return;
		}
		const nombre = nombreRom();
		descargar(rom, nombre);
		informar(`<div class="ok">[ok] ROM descargada</div><div class="dim">${escapar(nombre)} (${rom.length} bytes)</div>`);
	} finally {
		if (boton) {
			boton.disabled = false;
		}
	}
}
