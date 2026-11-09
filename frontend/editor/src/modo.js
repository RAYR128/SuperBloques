import {state, subscribe} from "./project.js";
import {redimensionarWorkspace} from "./blocks/workspace.js";

function tituloGfx(sel) {
	if (sel.gfx === "hud") {
		return `GFX Hud — ${sel.nombre}`;
	}
	if (sel.gfx === "objetos") {
		return "GFX Objetos";
	}
	return `GFX Principal — ${sel.nombre}`;
}

function aplicarModo() {
	const tipo = state.seleccion?.tipo;
	const modo = tipo === "musica" ? "musica" : tipo === "sonido" ? "sonido" : tipo === "gfx" ? "gfx" : "bloques";
	document.getElementById("blocks-panel").hidden = modo !== "bloques";
	document.getElementById("instruments-panel").hidden = modo !== "musica";
	document.getElementById("palette-panel").hidden = modo !== "gfx";
	document.getElementById("piano-roll").hidden = modo !== "musica";
	document.getElementById("sound-detail").hidden = modo !== "sonido";
	document.getElementById("gfx-editor").hidden = modo !== "gfx";
	document.querySelector(".workspace-body").dataset.modo = modo;
	const titulo = document.getElementById("workspace-title");
	if (modo === "bloques") {
		titulo.textContent = "Bloques";
		redimensionarWorkspace();
	} else if (modo === "gfx" && state.seleccion) {
		titulo.textContent = tituloGfx(state.seleccion);
	} else if (state.seleccion) {
		titulo.textContent = state.seleccion.nombre;
	} else {
		titulo.textContent = "Bloques";
	}
}

export function initModo() {
	subscribe(aplicarModo);
	aplicarModo();
}
