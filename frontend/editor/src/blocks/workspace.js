import * as Blockly from "blockly/core";
import * as Es from "blockly/msg/es";
import {
	agregarVariable,
	eliminarVariable,
	entidadActual,
	guardarBloquesActuales,
	state,
	subscribe,
} from "../project.js";
import {
	CATEGORIAS,
	categoriaDisponible,
	categoriaTieneBloques,
	flyoutDeCategoria,
	registrarBloques,
	theme,
} from "./catalog.js";
import {loadBloquesIntoWorkspace, workspaceToBloques} from "./serializer.js";

Blockly.setLocale(Es);

let workspace = null;
let categoria = "mocion";
let loading = false;
let flushTimer = 0;
let lastSelKey = null;

export function flushWorkspace() {
	clearTimeout(flushTimer);
	if (!workspace || loading) {
		return;
	}
	guardarBloquesActuales(workspaceToBloques(workspace));
}

function scheduleFlush() {
	if (loading) {
		return;
	}
	clearTimeout(flushTimer);
	flushTimer = setTimeout(flushWorkspace, 50);
}

function tipoSeleccion() {
	return state.seleccion?.tipo ?? null;
}

function toolboxJson() {
	return {
		kind: "flyoutToolbox",
		contents: flyoutDeCategoria(categoria, entidadActual(), tipoSeleccion()),
	};
}

function siguienteCategoria(tipo) {
	return (
		CATEGORIAS.find((cat) => categoriaDisponible(cat, tipo) && categoriaTieneBloques(cat, tipo)) ??
		CATEGORIAS.find((cat) => categoriaDisponible(cat, tipo))
	);
}

function syncCategorias() {
	const tipo = tipoSeleccion();
	const botones = document.querySelectorAll(".cat");
	botones.forEach((btn) => {
		const ok = categoriaDisponible(btn.dataset.cat, tipo);
		btn.disabled = !ok;
		if (ok) {
			btn.removeAttribute("title");
		} else {
			btn.title = tipo === "escena" ? "No disponible en escenas" : "No disponible en objetos";
		}
	});
	if (categoriaDisponible(categoria, tipo)) {
		return;
	}
	const siguiente = siguienteCategoria(tipo);
	if (!siguiente) {
		return;
	}
	categoria = siguiente;
	botones.forEach((btn) => btn.classList.toggle("active", btn.dataset.cat === siguiente));
}

function registrarCallbacksVariable() {
	if (!workspace) {
		return;
	}
	workspace.registerButtonCallback("crearVariable", () => {
		flushWorkspace();
		agregarVariable();
	});
	for (const nombre of entidadActual()?.Variables ?? []) {
		const key = `eliminarVariable:${nombre}`;
		workspace.registerButtonCallback(key, () => {
			flushWorkspace();
			eliminarVariable(nombre);
		});
	}
}

function refreshFlyout() {
	if (!workspace) {
		return;
	}
	syncCategorias();
	registrarCallbacksVariable();
	workspace.updateToolbox(toolboxJson());
}

function selKey() {
	const sel = state.seleccion;
	const gen = state.generation ?? 0;
	return sel ? `${gen}:${sel.tipo}:${sel.nombre}` : `${gen}:`;
}

function refreshVariableFields() {
	if (!workspace) {
		return;
	}
	for (const block of workspace.getAllBlocks(false)) {
		const field = block.getField("VAR");
		if (!field) {
			continue;
		}
		const value = field.getValue();
		if (typeof field.getOptions === "function") {
			field.getOptions(false);
		}
		try {
			field.setValue(value);
		} catch {
			/* la variable pudo desaparecer */
		}
		block.render();
	}
}

function loadEntityIntoWorkspace() {
	if (!workspace) {
		return;
	}
	clearTimeout(flushTimer);
	const key = selKey();
	if (key === lastSelKey) {
		refreshFlyout();
		refreshVariableFields();
		return;
	}
	lastSelKey = key;

	loading = true;
	workspace.clear();
	const entidad = entidadActual();
	if (entidad) {
		loadBloquesIntoWorkspace(workspace, entidad.Bloques || {}, entidad.Variables || []);
	}
	loading = false;
	refreshFlyout();
}

export function initWorkspace() {
	registrarBloques();
	syncCategorias();
	workspace = Blockly.inject("blocklyDiv", {
		renderer: "zelos",
		theme,
		toolbox: toolboxJson(),
		media: `${import.meta.env.BASE_URL}media/`,
		trashcan: true,
		sounds: false,
		zoom: {
			controls: true,
			wheel: true,
			startScale: 0.9,
			maxScale: 2,
			minScale: 0.3,
			scaleSpeed: 1.1,
		},
		grid: {spacing: 18, length: 1, colour: "#1f2636", snap: false},
		move: {scrollbars: true, drag: true, wheel: true},
	});

	workspace.addChangeListener((e) => {
		if (loading || e.isUiEvent) {
			return;
		}
		scheduleFlush();
	});

	document.querySelectorAll(".cat").forEach((btn) => {
		btn.addEventListener("click", () => {
			if (btn.disabled || !categoriaDisponible(btn.dataset.cat, tipoSeleccion())) {
				return;
			}
			document.querySelectorAll(".cat").forEach((b) => b.classList.remove("active"));
			btn.classList.add("active");
			categoria = btn.dataset.cat;
			refreshFlyout();
		});
	});

	const host = document.getElementById("blocklyDiv");
	const ro = new ResizeObserver(() => Blockly.svgResize(workspace));
	ro.observe(host);
	window.addEventListener("resize", () => Blockly.svgResize(workspace));

	subscribe(loadEntityIntoWorkspace);
	lastSelKey = null;
	loadEntityIntoWorkspace();
}
