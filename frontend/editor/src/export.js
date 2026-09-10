import {state} from "./project.js";

const NOMBRE_ARCHIVO = "Nuevo Proyecto.json";

export function exportarProyecto() {
	const json = JSON.stringify(state.proyecto, null, "\t");
	const blob = new Blob([json], {type: "application/json"});
	const url = URL.createObjectURL(blob);
	const a = document.createElement("a");
	a.href = url;
	a.download = NOMBRE_ARCHIVO;
	a.click();
	URL.revokeObjectURL(url);
}
