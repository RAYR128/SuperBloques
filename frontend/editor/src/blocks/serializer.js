import * as Blockly from "blockly/core";
import {STATEMENT_INPUTS, VALUE_INPUTS} from "./catalog.js";
import {state} from "../project.js";

const TIPOS_CONOCIDOS = new Set([
	"motion_get_posicion_x",
	"motion_get_posicion_y",
	"motion_set_posicion_x",
	"motion_set_posicion_y",
	"motion_add_posicion_x",
	"motion_add_posicion_y",
	"animacion_obj_set_sprite",
	"animacion_scene_set_mosaic_filter",
	"animacion_scene_set_brightness",
	"animacion_scene_set_color",
	"capa_layer1_position_x",
	"capa_layer1_position_y",
	"capa_layer2_position_x",
	"capa_layer2_position_y",
	"capa_layer3_position_x",
	"capa_layer3_position_y",
	"capa_set_layer1_position_x",
	"capa_set_layer1_position_y",
	"capa_set_layer2_position_x",
	"capa_set_layer2_position_y",
	"capa_set_layer3_position_x",
	"capa_set_layer3_position_y",
	"capa_set_tile_layer1",
	"capa_set_tile_layer2",
	"capa_set_tile_layer3",
	"control_if",
	"control_while",
	"control_ifelse",
	"evento_init",
	"evento_frame",
	"evento_estado",
	"evento_set_estado",
	"evento_cambiar_escena",
	"variable",
	"variable_store",
	"numero",
	"operation_add",
	"operation_sub",
	"operation_mul",
	"operation_div",
	"sensor_tiempo",
	"sensor_boton_control_1",
	"sensor_boton_control_2",
	"sensor_boton_control_1_presionado",
	"sensor_boton_control_2_presionado",
]);

function numeroPorDefecto() {
	return {Operacion: "numero", ParametroEspecial: 0, Entradas: []};
}

function fieldsToParam(node) {
	const f = node.fields || {};
	switch (node.type) {
		case "numero":
			return Number(f.NUM) || 0;
		case "variable":
		case "variable_store":
			return f.VAR ?? "";
		case "evento_frame":
			return Number(f.STATUS) || 0;
		case "evento_cambiar_escena":
			return f.ESCENA ?? "";
		case "sensor_boton_control_1":
		case "sensor_boton_control_2":
		case "sensor_boton_control_1_presionado":
		case "sensor_boton_control_2_presionado":
			return Number(f.BOTON) || 0;
		default:
			return 0;
	}
}

function resolveVarName(param, variables) {
	if (typeof param === "string") {
		return param;
	}
	if (typeof param === "number" && variables[param] != null) {
		return variables[param];
	}
	return variables[0] ?? "";
}

function resolveEscenaName(param) {
	const names = Object.keys(state.proyecto?.Escenas ?? {});
	if (typeof param === "string") {
		return names.includes(param) ? param : (names[0] ?? "");
	}
	if (typeof param === "number" && names[param] != null) {
		return names[param];
	}
	return names[0] ?? "";
}

function paramToFields(type, param, variables) {
	switch (type) {
		case "numero":
			return {NUM: Number(param) || 0};
		case "variable":
		case "variable_store":
			return {VAR: resolveVarName(param, variables)};
		case "evento_frame":
			return {STATUS: Number(param) || 0};
		case "evento_cambiar_escena":
			return {ESCENA: resolveEscenaName(param)};
		case "sensor_boton_control_1":
		case "sensor_boton_control_2":
		case "sensor_boton_control_1_presionado":
		case "sensor_boton_control_2_presionado":
			return {BOTON: String(Number(param) || 0)};
		default:
			return {};
	}
}

function slotNode(slot) {
	if (!slot) {
		return null;
	}
	return slot.block || slot.shadow || null;
}

function serializeValue(slot) {
	const node = slotNode(slot);
	if (!node || !TIPOS_CONOCIDOS.has(node.type)) {
		return numeroPorDefecto();
	}
	return {
		Operacion: node.type,
		ParametroEspecial: fieldsToParam(node),
		Entradas: valueEntradas(node),
	};
}

function valueEntradas(node) {
	const names = VALUE_INPUTS[node.type] || [];
	return names.map((name) => serializeValue(node.inputs?.[name]));
}

function collectStatement(node, acc, seen) {
	if (!node || seen.has(node)) {
		return;
	}
	seen.add(node);
	acc.push(node);
	for (const name of STATEMENT_INPUTS[node.type] || []) {
		collectStatement(slotNode(node.inputs?.[name]), acc, seen);
	}
	collectStatement(node.next?.block, acc, seen);
}

function idDe(indice) {
	return `bloque_id_${String(indice + 1).padStart(3, "0")}`;
}

export function blocklySaveToBloques(save) {
	const tops = [...(save?.blocks?.blocks ?? [])];
	tops.sort((a, b) => (a.y ?? 0) - (b.y ?? 0) || (a.x ?? 0) - (b.x ?? 0));

	const acc = [];
	const seen = new Set();
	for (const top of tops) {
		collectStatement(top, acc, seen);
	}

	const idMap = new Map();
	acc.forEach((node, i) => idMap.set(node, idDe(i)));

	const bloques = {};
	for (const node of acc) {
		if (!TIPOS_CONOCIDOS.has(node.type)) {
			console.warn("Operacion desconocida, se omite:", node.type);
			continue;
		}
		const id = idMap.get(node);
		const entrada = {
			PosicionVisual: [Math.round(node.x ?? 0), Math.round(node.y ?? 0)],
			Operacion: node.type,
			ParametroEspecial: fieldsToParam(node),
			Entradas: valueEntradas(node),
			Siguiente: null,
			Previo: null,
		};
		const next = node.next?.block;
		if (next && idMap.has(next)) {
			entrada.Siguiente = idMap.get(next);
		}
		const cuerpo = slotNode(node.inputs?.CUERPO);
		if (cuerpo && idMap.has(cuerpo)) {
			entrada.Cuerpo = idMap.get(cuerpo);
		}
		const cuerpoSino = slotNode(node.inputs?.CUERPO_SINO);
		if (cuerpoSino && idMap.has(cuerpoSino)) {
			entrada.CuerpoSino = idMap.get(cuerpoSino);
		}
		bloques[id] = entrada;
	}

	for (const [id, entrada] of Object.entries(bloques)) {
		if (entrada.Siguiente && bloques[entrada.Siguiente]) {
			bloques[entrada.Siguiente].Previo = id;
		}
	}

	return bloques;
}

export function workspaceToBloques(workspace) {
	return blocklySaveToBloques(Blockly.serialization.workspaces.save(workspace));
}

function nestedToBlockly(nodo, variables) {
	if (!nodo || !TIPOS_CONOCIDOS.has(nodo.Operacion)) {
		return null;
	}
	const block = {
		type: nodo.Operacion,
		fields: paramToFields(nodo.Operacion, nodo.ParametroEspecial, variables),
	};
	const names = VALUE_INPUTS[nodo.Operacion] || [];
	if (names.length) {
		block.inputs = {};
		names.forEach((name, i) => {
			const child = nestedToBlockly(nodo.Entradas?.[i], variables);
			if (child) {
				block.inputs[name] = {block: child};
			}
		});
	}
	return block;
}

function destinosCuerpo(bloques) {
	const s = new Set();
	for (const b of Object.values(bloques)) {
		if (b.Cuerpo) {
			s.add(b.Cuerpo);
		}
		if (b.CuerpoSino) {
			s.add(b.CuerpoSino);
		}
	}
	return s;
}

function chainToBlockly(id, bloques, variables, withPos, visited) {
	if (!id || !bloques[id] || visited.has(id)) {
		return null;
	}
	const b = bloques[id];
	if (!TIPOS_CONOCIDOS.has(b.Operacion)) {
		console.warn("Operacion desconocida, se omite:", b.Operacion);
		visited.add(id);
		return chainToBlockly(b.Siguiente, bloques, variables, withPos, visited);
	}
	visited.add(id);

	const node = {
		type: b.Operacion,
		id,
		fields: paramToFields(b.Operacion, b.ParametroEspecial, variables),
	};
	if (withPos && Array.isArray(b.PosicionVisual) && b.PosicionVisual.length >= 2) {
		node.x = b.PosicionVisual[0];
		node.y = b.PosicionVisual[1];
	}

	const vNames = VALUE_INPUTS[b.Operacion] || [];
	const sNames = STATEMENT_INPUTS[b.Operacion] || [];
	if (vNames.length || sNames.length) {
		node.inputs = {};
	}
	vNames.forEach((name, i) => {
		const child = nestedToBlockly(b.Entradas?.[i], variables);
		if (child) {
			node.inputs[name] = {block: child};
		}
	});
	if (sNames.includes("CUERPO") && b.Cuerpo) {
		const inner = chainToBlockly(b.Cuerpo, bloques, variables, false, visited);
		if (inner) {
			node.inputs.CUERPO = {block: inner};
		}
	}
	if (sNames.includes("CUERPO_SINO") && b.CuerpoSino) {
		const inner = chainToBlockly(b.CuerpoSino, bloques, variables, false, visited);
		if (inner) {
			node.inputs.CUERPO_SINO = {block: inner};
		}
	}
	if (b.Siguiente) {
		const next = chainToBlockly(b.Siguiente, bloques, variables, false, visited);
		if (next) {
			node.next = {block: next};
		}
	}
	return node;
}

export function bloquesToWorkspace(bloques, variables = []) {
	const cuerpo = destinosCuerpo(bloques);
	const visited = new Set();
	const roots = [];

	const ids = Object.keys(bloques);
	ids.sort((a, b) => {
		const pa = bloques[a].PosicionVisual || [0, 0];
		const pb = bloques[b].PosicionVisual || [0, 0];
		return (pa[1] ?? 0) - (pb[1] ?? 0) || (pa[0] ?? 0) - (pb[0] ?? 0);
	});

	for (const id of ids) {
		const b = bloques[id];
		const esRaiz = !b.Previo && !cuerpo.has(id);
		if (!esRaiz) {
			continue;
		}
		const node = chainToBlockly(id, bloques, variables, true, visited);
		if (node) {
			roots.push(node);
		}
	}

	for (const id of ids) {
		if (visited.has(id)) {
			continue;
		}
		const node = chainToBlockly(id, bloques, variables, true, visited);
		if (node) {
			roots.push(node);
		}
	}

	return {
		blocks: {
			languageVersion: 0,
			blocks: roots,
		},
	};
}

export function loadBloquesIntoWorkspace(workspace, bloques, variables) {
	const state = bloquesToWorkspace(bloques, variables);
	Blockly.serialization.workspaces.load(state, workspace);
}
