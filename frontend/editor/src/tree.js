import {
	agregarEscena,
	agregarObjeto,
	eliminarEntidad,
	seleccionar,
	state,
	subscribe,
} from "./project.js";
import {flushWorkspace} from "./blocks/workspace.js";

function itemEl(tipo, nombre) {
	const sel = state.seleccion;
	const active = sel && sel.tipo === tipo && sel.nombre === nombre;
	const row = document.createElement("div");
	row.className = "tree-item" + (active ? " active" : "");
	row.dataset.tipo = tipo;
	row.dataset.nombre = nombre;

	const name = document.createElement("div");
	name.className = "item-name";
	const dot = document.createElement("span");
	dot.className = "dot" + (tipo === "escena" ? " scene" : "");
	const label = document.createElement("span");
	label.textContent = nombre;
	name.append(dot, label);

	const remove = document.createElement("button");
	remove.type = "button";
	remove.className = "tree-remove";
	remove.title = "Eliminar";
	remove.textContent = "×";
	remove.addEventListener("click", (e) => {
		e.stopPropagation();
		flushWorkspace();
		eliminarEntidad(tipo, nombre);
	});

	row.append(name, remove);
	row.addEventListener("click", () => {
		if (active) {
			return;
		}
		flushWorkspace();
		seleccionar(tipo, nombre);
	});
	return row;
}

function renderLista(el, tipo, mapa) {
	el.replaceChildren();
	for (const nombre of Object.keys(mapa)) {
		el.append(itemEl(tipo, nombre));
	}
}

export function renderTree() {
	renderLista(document.getElementById("tree-escenas"), "escena", state.proyecto.Escenas);
	renderLista(document.getElementById("tree-objetos"), "objeto", state.proyecto.Objetos);
}

export function initTree() {
	document.getElementById("add-escena").addEventListener("click", () => {
		flushWorkspace();
		agregarEscena();
	});
	document.getElementById("add-objeto").addEventListener("click", () => {
		flushWorkspace();
		agregarObjeto();
	});
	subscribe(renderTree);
	renderTree();
}
