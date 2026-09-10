import {NOMBRE_ARCHIVO_DEFECTO, state} from "./project.js";

export function exportarProyecto() {
	const json = JSON.stringify(state.proyecto, null, "\t");
	const blob = new Blob([json], {type: "application/json"});
	const url = URL.createObjectURL(blob);
	const a = document.createElement("a");
	a.href = url;
	a.download = state.nombreArchivo || NOMBRE_ARCHIVO_DEFECTO;
	a.click();
	URL.revokeObjectURL(url);
}
