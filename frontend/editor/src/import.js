import {parsearProyecto} from "./formato.js";
import {cargarProyecto} from "./project.js";

function informar(html) {
	const el = document.querySelector(".debug-body");
	if (el) {
		el.innerHTML = html;
	}
}

function elegirArchivo() {
	const input = document.getElementById("menu-import-file");
	if (!input) {
		return Promise.resolve(null);
	}
	return new Promise((resolve) => {
		let done = false;
		const terminar = (archivo) => {
			if (done) {
				return;
			}
			done = true;
			input.removeEventListener("change", onChange);
			input.removeEventListener("cancel", onCancel);
			input.value = "";
			resolve(archivo);
		};
		const onChange = () => terminar(input.files?.[0] ?? null);
		const onCancel = () => terminar(null);
		input.addEventListener("change", onChange);
		input.addEventListener("cancel", onCancel);
		input.click();
	});
}

export async function importarProyecto() {
	const archivo = await elegirArchivo();
	if (!archivo) {
		return false;
	}

	let texto;
	try {
		texto = await archivo.text();
	} catch (e) {
		const msg = e instanceof Error ? e.message : String(e);
		informar(`<div class="warn">[error] no se pudo leer el archivo</div><div class="dim">${msg}</div>`);
		window.alert("No se pudo leer el archivo");
		return false;
	}

	let proyecto;
	try {
		proyecto = parsearProyecto(texto);
	} catch (e) {
		const msg = e instanceof Error ? e.message : String(e);
		informar(`<div class="warn">[error] ${msg}</div><div class="dim">el proyecto en memoria no se modifico</div>`);
		window.alert("No se pudo importar: " + msg);
		return false;
	}

	cargarProyecto(proyecto, archivo.name);
	informar(`<div class="ok">[ok] proyecto importado</div><div class="dim">${archivo.name}</div>`);
	return true;
}
