import {
	agregarEscena,
	agregarMusica,
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
	const marca = tipo === "escena" ? " scene" : tipo === "sonido" ? " sound" : tipo === "musica" ? " music" : "";
	dot.className = "dot" + marca;
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

function gfxActivo(gfx, nombre) {
	const sel = state.seleccion;
	return !!(sel && sel.tipo === "gfx" && sel.gfx === gfx && sel.nombre === nombre);
}

function filaGfx(etiqueta, gfx, nombre) {
	const row = document.createElement("button");
	row.type = "button";
	row.className = "tree-gfx" + (gfxActivo(gfx, nombre) ? " active" : "");
	row.dataset.tipo = "gfx";
	row.dataset.gfx = gfx;
	row.textContent = etiqueta;
	row.addEventListener("click", (e) => {
		e.stopPropagation();
		if (gfxActivo(gfx, nombre)) {
			return;
		}
		flushWorkspace();
		seleccionar("gfx", nombre, gfx);
	});
	return row;
}

function escenaAbierta(nombre) {
	const sel = state.seleccion;
	if (!sel) {
		return false;
	}
	if (sel.tipo === "escena" && sel.nombre === nombre) {
		return true;
	}
	return sel.tipo === "gfx" && sel.gfx !== "objetos" && sel.nombre === nombre;
}

function escenaEl(nombre) {
	const wrap = document.createElement("div");
	wrap.className = "tree-scene";
	wrap.append(itemEl("escena", nombre));
	if (!escenaAbierta(nombre)) {
		return wrap;
	}
	const sub = document.createElement("div");
	sub.className = "tree-sub";
	sub.append(filaGfx("GFX Principal", "principal", nombre), filaGfx("GFX Hud", "hud", nombre));
	wrap.append(sub);
	return wrap;
}

function renderLista(el, tipo, mapa) {
	el.replaceChildren();
	if (tipo === "objeto") {
		el.append(filaGfx("GFX Objetos", "objetos", ""));
	}
	for (const nombre of Object.keys(mapa)) {
		el.append(tipo === "escena" ? escenaEl(nombre) : itemEl(tipo, nombre));
	}
}

export function renderTree() {
	renderLista(document.getElementById("tree-escenas"), "escena", state.proyecto.Escenas);
	renderLista(document.getElementById("tree-objetos"), "objeto", state.proyecto.Objetos);
	renderLista(document.getElementById("tree-sonidos"), "sonido", state.proyecto.Sonidos);
	renderLista(document.getElementById("tree-musicas"), "musica", state.proyecto.Musica);
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
	document.getElementById("add-musica").addEventListener("click", () => {
		flushWorkspace();
		agregarMusica();
	});
	subscribe(renderTree);
	renderTree();
}
