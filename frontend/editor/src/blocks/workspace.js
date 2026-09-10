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
import {flyoutDeCategoria, registrarBloques, theme} from "./catalog.js";
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

function toolboxJson() {
	return {
		kind: "flyoutToolbox",
		contents: flyoutDeCategoria(categoria, entidadActual()),
	};
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
	registrarCallbacksVariable();
	workspace.updateToolbox(toolboxJson());
}

function selKey() {
	const sel = state.seleccion;
	return sel ? `${sel.tipo}:${sel.nombre}` : "";
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
