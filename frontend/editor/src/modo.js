import {state, subscribe} from "./project.js";
import {redimensionarWorkspace} from "./blocks/workspace.js";

function aplicarModo() {
	const tipo = state.seleccion?.tipo;
	const modo = tipo === "musica" ? "musica" : tipo === "sonido" ? "sonido" : "bloques";
	document.getElementById("blocks-panel").hidden = modo !== "bloques";
	document.getElementById("instruments-panel").hidden = modo !== "musica";
	document.getElementById("piano-roll").hidden = modo !== "musica";
	document.getElementById("sound-detail").hidden = modo !== "sonido";
	document.querySelector(".workspace-body").dataset.modo = modo;
	const titulo = document.getElementById("workspace-title");
	if (modo === "bloques") {
		titulo.textContent = "Bloques";
		redimensionarWorkspace();
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
