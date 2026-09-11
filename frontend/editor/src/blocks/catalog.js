import * as Blockly from "blockly/core";
import {state, variableEnUso, variablesActuales} from "../project.js";

export const VALUE_INPUTS = {
	motion_set_posicion_x: ["VAL"],
	motion_set_posicion_y: ["VAL"],
	motion_add_posicion_x: ["VAL"],
	motion_add_posicion_y: ["VAL"],
	animacion_obj_set_sprite: ["VAL"],
	animacion_scene_set_mosaic_filter: ["VAL"],
	animacion_scene_set_brightness: ["VAL"],
	animacion_scene_set_color: ["IDX", "R", "G", "B"],
	capa_set_layer1_position_x: ["VAL"],
	capa_set_layer1_position_y: ["VAL"],
	capa_set_layer2_position_x: ["VAL"],
	capa_set_layer2_position_y: ["VAL"],
	capa_set_layer3_position_x: ["VAL"],
	capa_set_layer3_position_y: ["VAL"],
	capa_set_tile_layer1: ["X", "Y", "TILE", "PALETA", "FLIPX", "FLIPY", "PRIO"],
	capa_set_tile_layer2: ["X", "Y", "TILE", "PALETA", "FLIPX", "FLIPY", "PRIO"],
	capa_set_tile_layer3: ["X", "Y", "TILE", "PALETA", "FLIPX", "FLIPY", "PRIO"],
	variable_store: ["VAL"],
	operation_add: ["A", "B"],
	operation_sub: ["A", "B"],
	operation_mul: ["A", "B"],
	operation_div: ["A", "B"],
	control_if: ["COND"],
	control_while: ["COND"],
	control_ifelse: ["COND"],
	evento_set_estado: ["VAL"],
};

export const STATEMENT_INPUTS = {
	control_if: ["CUERPO"],
	control_while: ["CUERPO"],
	control_ifelse: ["CUERPO", "CUERPO_SINO"],
};

export const BOTONES = [
	/* TO-DO: es necesario tener esto? no se si los otros controles de la consola (mouse, ej) utilizan esto para identificarlos
	pero solo estamos usando el joypad normal y es muy dudoso que añada soporte al periferico del mouse o otros controles.
	["Firma 0", "0"],
	["Firma 1", "1"],
	["Firma 2", "2"],
	["Firma 3", "3"],*/
	["R", "4"],
	["L", "5"],
	["X", "6"],
	["A", "7"],
	["Derecha", "8"],
	["Izquierda", "9"],
	["Abajo", "10"],
	["Arriba", "11"],
	["Start", "12"],
	["Select", "13"],
	["Y", "14"],
	["B", "15"],
];

const COLORES = {
	mocion: "#4c97ff",
	animacion: "#9966ff",
	capa: "#00c2a8",
	sonido: "#cf63cf",
	control: "#ff194b",
	evento: "#ffbf00",
	variable: "#ff8c1a",
	operacion: "#59c059",
	sensor: "#5cb1d6",
};

function shade(hex, amt) {
	const n = parseInt(hex.slice(1), 16);
	const adj = (c) => Math.max(0, Math.min(255, c + amt));
	const r = adj((n >> 16) & 255);
	const g = adj((n >> 8) & 255);
	const b = adj(n & 255);
	return `#${((r << 16) | (g << 8) | b).toString(16).padStart(6, "0")}`;
}

function style(hex) {
	return {
		colourPrimary: hex,
		colourSecondary: shade(hex, -18),
		colourTertiary: shade(hex, -40),
	};
}

export const theme = Blockly.Theme.defineTheme("superbloques", {
	name: "superbloques",
	blockStyles: {
		mocion_blocks: style(COLORES.mocion),
		animacion_blocks: style(COLORES.animacion),
		capa_blocks: style(COLORES.capa),
		sonido_blocks: style(COLORES.sonido),
		control_blocks: style(COLORES.control),
		evento_blocks: style(COLORES.evento),
		variable_blocks: style(COLORES.variable),
		operacion_blocks: style(COLORES.operacion),
		sensor_blocks: style(COLORES.sensor),
	},
	componentStyles: {
		workspaceBackgroundColour: "#141820",
		toolboxBackgroundColour: "#1a1f2b",
		toolboxForegroundColour: "#e8edf5",
		flyoutBackgroundColour: "#1a1f2b",
		flyoutForegroundColour: "#e8edf5",
		flyoutOpacity: 1,
		scrollbarColour: "#3a4458",
		insertionMarkerColour: "#7aa2ff",
		insertionMarkerOpacity: 0.3,
		scrollbarOpacity: 0.7,
		cursorColour: "#7aa2ff",
	},
	fontStyle: {
		family: "Courier New, Courier, monospace",
		weight: "bold",
		size: 11,
	},
	startHats: true,
});

function shadowNumero(n = 0) {
	return {shadow: {type: "numero", fields: {NUM: n}}};
}

function variableOptions() {
	const names = variablesActuales();
	if (!names.length) {
		return [["(sin variables)", ""]];
	}
	return names.map((n) => [n, n]);
}

function escenaOptions() {
	const names = Object.keys(state.proyecto?.Escenas ?? {});
	if (!names.length) {
		return [["(sin escenas)", ""]];
	}
	return names.map((n) => [n, n]);
}

const JSON_BLOCKS = [
	{
		type: "motion_get_posicion_x",
		message0: "posicion x",
		output: "Number",
		style: "mocion_blocks",
	},
	{
		type: "motion_get_posicion_y",
		message0: "posicion y",
		output: "Number",
		style: "mocion_blocks",
	},
	{
		type: "motion_set_posicion_x",
		message0: "fijar posicion x a %1",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "mocion_blocks",
	},
	{
		type: "motion_set_posicion_y",
		message0: "fijar posicion y a %1",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "mocion_blocks",
	},
	{
		type: "motion_add_posicion_x",
		message0: "cambiar posicion x por %1",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "mocion_blocks",
	},
	{
		type: "motion_add_posicion_y",
		message0: "cambiar posicion y por %1",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "mocion_blocks",
	},
	{
		type: "animacion_obj_set_sprite",
		message0: "fijar sprite a %1",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "animacion_blocks",
	},
	{
		type: "animacion_scene_set_mosaic_filter",
		message0: "cambiar filtro mosaico a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "animacion_blocks",
	},
	{
		type: "animacion_scene_set_brightness",
		message0: "cambiar brillo a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "animacion_blocks",
	},
	{
		type: "animacion_scene_set_color",
		message0: "cambiar color paleta %1 a R %2 G %3 B %4 en escena",
		args0: [
			{type: "input_value", name: "IDX", check: "Number"},
			{type: "input_value", name: "R", check: "Number"},
			{type: "input_value", name: "G", check: "Number"},
			{type: "input_value", name: "B", check: "Number"},
		],
		previousStatement: null,
		nextStatement: null,
		style: "animacion_blocks",
	},
	{
		type: "capa_layer1_position_x",
		message0: "posicion capa 1 x en escena",
		output: "Number",
		style: "capa_blocks",
	},
	{
		type: "capa_layer1_position_y",
		message0: "posicion capa 1 y en escena",
		output: "Number",
		style: "capa_blocks",
	},
	{
		type: "capa_layer2_position_x",
		message0: "posicion capa 2 x en escena",
		output: "Number",
		style: "capa_blocks",
	},
	{
		type: "capa_layer2_position_y",
		message0: "posicion capa 2 y en escena",
		output: "Number",
		style: "capa_blocks",
	},
	{
		type: "capa_layer3_position_x",
		message0: "posicion capa 3 x en escena",
		output: "Number",
		style: "capa_blocks",
	},
	{
		type: "capa_layer3_position_y",
		message0: "posicion capa 3 y en escena",
		output: "Number",
		style: "capa_blocks",
	},
	{
		type: "capa_set_layer1_position_x",
		message0: "fijar posicion capa 1 x a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_layer1_position_y",
		message0: "fijar posicion capa 1 y a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_layer2_position_x",
		message0: "fijar posicion capa 2 x a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_layer2_position_y",
		message0: "fijar posicion capa 2 y a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_layer3_position_x",
		message0: "fijar posicion capa 3 x a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_layer3_position_y",
		message0: "fijar posicion capa 3 y a %1 en escena",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_tile_layer1",
		message0: "poner tile capa 1 x %1 y %2 tile %3 paleta %4 flip x %5 flip y %6 prioridad %7",
		args0: [
			{type: "input_value", name: "X", check: "Number"},
			{type: "input_value", name: "Y", check: "Number"},
			{type: "input_value", name: "TILE", check: "Number"},
			{type: "input_value", name: "PALETA", check: "Number"},
			{type: "input_value", name: "FLIPX", check: "Number"},
			{type: "input_value", name: "FLIPY", check: "Number"},
			{type: "input_value", name: "PRIO", check: "Number"},
		],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_tile_layer2",
		message0: "poner tile capa 2 x %1 y %2 tile %3 paleta %4 flip x %5 flip y %6 prioridad %7",
		args0: [
			{type: "input_value", name: "X", check: "Number"},
			{type: "input_value", name: "Y", check: "Number"},
			{type: "input_value", name: "TILE", check: "Number"},
			{type: "input_value", name: "PALETA", check: "Number"},
			{type: "input_value", name: "FLIPX", check: "Number"},
			{type: "input_value", name: "FLIPY", check: "Number"},
			{type: "input_value", name: "PRIO", check: "Number"},
		],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "capa_set_tile_layer3",
		message0: "poner tile capa 3 x %1 y %2 tile %3 paleta %4 flip x %5 flip y %6 prioridad %7",
		args0: [
			{type: "input_value", name: "X", check: "Number"},
			{type: "input_value", name: "Y", check: "Number"},
			{type: "input_value", name: "TILE", check: "Number"},
			{type: "input_value", name: "PALETA", check: "Number"},
			{type: "input_value", name: "FLIPX", check: "Number"},
			{type: "input_value", name: "FLIPY", check: "Number"},
			{type: "input_value", name: "PRIO", check: "Number"},
		],
		previousStatement: null,
		nextStatement: null,
		style: "capa_blocks",
	},
	{
		type: "control_if",
		message0: "si %1 no es 0 entonces",
		args0: [{type: "input_value", name: "COND", check: "Number"}],
		message1: "%1",
		args1: [{type: "input_statement", name: "CUERPO"}],
		previousStatement: null,
		nextStatement: null,
		style: "control_blocks",
	},
	{
		type: "control_while",
		message0: "mientras que %1 no es 0",
		args0: [{type: "input_value", name: "COND", check: "Number"}],
		message1: "%1",
		args1: [{type: "input_statement", name: "CUERPO"}],
		previousStatement: null,
		nextStatement: null,
		style: "control_blocks",
	},
	{
		type: "control_ifelse",
		message0: "si %1 no es 0 entonces",
		args0: [{type: "input_value", name: "COND", check: "Number"}],
		message1: "%1",
		args1: [{type: "input_statement", name: "CUERPO"}],
		message2: "si no",
		message3: "%1",
		args3: [{type: "input_statement", name: "CUERPO_SINO"}],
		previousStatement: null,
		nextStatement: null,
		style: "control_blocks",
	},
	{
		type: "evento_init",
		message0: "al iniciar",
		nextStatement: null,
		style: "evento_blocks",
		hat: "cap",
	},
	{
		type: "evento_frame",
		message0: "cada cuadro (estado %1)",
		args0: [
			{
				type: "field_number",
				name: "STATUS",
				value: 0,
				min: 0,
				max: 255,
				precision: 1,
			},
		],
		nextStatement: null,
		style: "evento_blocks",
		hat: "cap",
	},
	{
		type: "evento_estado",
		message0: "estado",
		output: "Number",
		style: "evento_blocks",
	},
	{
		type: "evento_set_estado",
		message0: "fijar estado a %1",
		args0: [{type: "input_value", name: "VAL", check: "Number"}],
		previousStatement: null,
		nextStatement: null,
		style: "evento_blocks",
	},
	{
		type: "numero",
		message0: "%1",
		args0: [
			{
				type: "field_number",
				name: "NUM",
				value: 0,
				min: -32768,
				max: 65535,
				precision: 1,
			},
		],
		output: "Number",
		style: "operacion_blocks",
	},
	{
		type: "operation_add",
		message0: "%1 + %2",
		args0: [
			{type: "input_value", name: "A", check: "Number"},
			{type: "input_value", name: "B", check: "Number"},
		],
		inputsInline: true,
		output: "Number",
		style: "operacion_blocks",
	},
	{
		type: "operation_sub",
		message0: "%1 − %2",
		args0: [
			{type: "input_value", name: "A", check: "Number"},
			{type: "input_value", name: "B", check: "Number"},
		],
		inputsInline: true,
		output: "Number",
		style: "operacion_blocks",
	},
	{
		type: "operation_mul",
		message0: "%1 × %2",
		args0: [
			{type: "input_value", name: "A", check: "Number"},
			{type: "input_value", name: "B", check: "Number"},
		],
		inputsInline: true,
		output: "Number",
		style: "operacion_blocks",
	},
	{
		type: "operation_div",
		message0: "%1 ÷ %2",
		args0: [
			{type: "input_value", name: "A", check: "Number"},
			{type: "input_value", name: "B", check: "Number"},
		],
		inputsInline: true,
		output: "Number",
		style: "operacion_blocks",
	},
	{
		type: "sensor_tiempo",
		message0: "tiempo",
		output: "Number",
		style: "sensor_blocks",
	},
	{
		type: "sensor_boton_control_1",
		message0: "boton %1 control 1",
		args0: [{type: "field_dropdown", name: "BOTON", options: BOTONES}],
		output: "Number",
		style: "sensor_blocks",
	},
	{
		type: "sensor_boton_control_2",
		message0: "boton %1 control 2",
		args0: [{type: "field_dropdown", name: "BOTON", options: BOTONES}],
		output: "Number",
		style: "sensor_blocks",
	},
	{
		type: "sensor_boton_control_1_presionado",
		message0: "boton %1 control 1 presionado",
		args0: [{type: "field_dropdown", name: "BOTON", options: BOTONES}],
		output: "Number",
		style: "sensor_blocks",
	},
	{
		type: "sensor_boton_control_2_presionado",
		message0: "boton %1 control 2 presionado",
		args0: [{type: "field_dropdown", name: "BOTON", options: BOTONES}],
		output: "Number",
		style: "sensor_blocks",
	},
];

export function registrarBloques() {
	Blockly.defineBlocksWithJsonArray(JSON_BLOCKS);

	Blockly.Blocks.variable = {
		init() {
			this.appendDummyInput().appendField(new Blockly.FieldDropdown(variableOptions), "VAR");
			this.setOutput(true, "Number");
			this.setStyle("variable_blocks");
		},
	};

	Blockly.Blocks.variable_store = {
		init() {
			this.appendDummyInput()
				.appendField("fijar")
				.appendField(new Blockly.FieldDropdown(variableOptions), "VAR")
				.appendField("a");
			this.appendValueInput("VAL").setCheck("Number");
			this.setPreviousStatement(true, null);
			this.setNextStatement(true, null);
			this.setInputsInline(true);
			this.setStyle("variable_blocks");
		},
	};

	Blockly.Blocks.evento_cambiar_escena = {
		init() {
			this.appendDummyInput()
				.appendField("cambiar a escena")
				.appendField(new Blockly.FieldDropdown(escenaOptions), "ESCENA");
			this.setPreviousStatement(true, null);
			this.setStyle("evento_blocks");
		},
	};
}

function bloqueConSombras(type, inputs) {
	const item = {kind: "block", type};
	if (inputs) {
		item.inputs = inputs;
	}
	return item;
}

// Bloques que no se ofrecen en el toolbox segun el tipo de entidad.
// La mocion es de objetos (una escena no tiene posicion propia).
// Los bloques de animacion van a ser especificos de objeto o de escena
// (p.ej. animacion_obj_set_sprite solo en objetos).
export const BLOQUES_NO_PERMITIDOS = {
	escena: new Set([
		"motion_get_posicion_x",
		"motion_get_posicion_y",
		"motion_set_posicion_x",
		"motion_set_posicion_y",
		"motion_add_posicion_x",
		"motion_add_posicion_y",
		"animacion_obj_set_sprite",
	]),
	objeto: new Set([
		// bloques especificos a la escena, cuando existan
	]),
};

export function bloquePermitido(type, tipo) {
	if (!type || !tipo) {
		return false;
	}
	return !BLOQUES_NO_PERMITIDOS[tipo]?.has(type);
}

function filtrarFlyout(items, tipo) {
	const filtrados = items.filter((item) => item.kind !== "block" || bloquePermitido(item.type, tipo));
	if (!filtrados.some((item) => item.kind === "block" || item.kind === "button")) {
		return [{kind: "label", text: "sin bloques"}];
	}
	return filtrados;
}

export function categoriaDisponible(cat, tipo) {
	if (cat === "variable") {
		return true;
	}
	const items = FLYOUT[cat] ?? [];
	const bloques = items.filter((item) => item.kind === "block");
	if (!bloques.length) {
		return true;
	}
	return bloques.some((item) => bloquePermitido(item.type, tipo));
}

export function categoriaTieneBloques(cat, tipo) {
	if (cat === "variable") {
		return true;
	}
	const items = filtrarFlyout(FLYOUT[cat] ?? [], tipo);
	return items.some((item) => item.kind === "block" || item.kind === "button");
}

const FLYOUT = {
	mocion: [
		{kind: "block", type: "motion_get_posicion_x"},
		{kind: "block", type: "motion_get_posicion_y"},
		bloqueConSombras("motion_set_posicion_x", {VAL: shadowNumero()}),
		bloqueConSombras("motion_set_posicion_y", {VAL: shadowNumero()}),
		bloqueConSombras("motion_add_posicion_x", {VAL: shadowNumero()}),
		bloqueConSombras("motion_add_posicion_y", {VAL: shadowNumero()}),
	],
	animacion: [
		bloqueConSombras("animacion_obj_set_sprite", {VAL: shadowNumero()}),
		bloqueConSombras("animacion_scene_set_mosaic_filter", {VAL: shadowNumero()}),
		bloqueConSombras("animacion_scene_set_brightness", {VAL: shadowNumero()}),
		bloqueConSombras("animacion_scene_set_color", {
			IDX: shadowNumero(),
			R: shadowNumero(),
			G: shadowNumero(),
			B: shadowNumero(),
		}),
	],
	capa: [
		{kind: "block", type: "capa_layer1_position_x"},
		{kind: "block", type: "capa_layer1_position_y"},
		{kind: "block", type: "capa_layer2_position_x"},
		{kind: "block", type: "capa_layer2_position_y"},
		{kind: "block", type: "capa_layer3_position_x"},
		{kind: "block", type: "capa_layer3_position_y"},
		bloqueConSombras("capa_set_layer1_position_x", {VAL: shadowNumero()}),
		bloqueConSombras("capa_set_layer1_position_y", {VAL: shadowNumero()}),
		bloqueConSombras("capa_set_layer2_position_x", {VAL: shadowNumero()}),
		bloqueConSombras("capa_set_layer2_position_y", {VAL: shadowNumero()}),
		bloqueConSombras("capa_set_layer3_position_x", {VAL: shadowNumero()}),
		bloqueConSombras("capa_set_layer3_position_y", {VAL: shadowNumero()}),
		bloqueConSombras("capa_set_tile_layer1", {
			X: shadowNumero(),
			Y: shadowNumero(),
			TILE: shadowNumero(),
			PALETA: shadowNumero(),
			FLIPX: shadowNumero(),
			FLIPY: shadowNumero(),
			PRIO: shadowNumero(),
		}),
		bloqueConSombras("capa_set_tile_layer2", {
			X: shadowNumero(),
			Y: shadowNumero(),
			TILE: shadowNumero(),
			PALETA: shadowNumero(),
			FLIPX: shadowNumero(),
			FLIPY: shadowNumero(),
			PRIO: shadowNumero(),
		}),
		bloqueConSombras("capa_set_tile_layer3", {
			X: shadowNumero(),
			Y: shadowNumero(),
			TILE: shadowNumero(),
			PALETA: shadowNumero(),
			FLIPX: shadowNumero(),
			FLIPY: shadowNumero(),
			PRIO: shadowNumero(),
		}),
	],
	sonido: [{kind: "label", text: "sin bloques"}],
	control: [
		bloqueConSombras("control_if", {COND: shadowNumero()}),
		bloqueConSombras("control_while", {COND: shadowNumero()}),
		bloqueConSombras("control_ifelse", {COND: shadowNumero()}),
	],
	evento: [
		{kind: "block", type: "evento_init"},
		{kind: "block", type: "evento_frame"},
		{kind: "block", type: "evento_estado"},
		bloqueConSombras("evento_set_estado", {VAL: shadowNumero()}),
		{kind: "block", type: "evento_cambiar_escena"},
	],
	operacion: [
		{kind: "block", type: "numero"},
		bloqueConSombras("operation_add", {A: shadowNumero(), B: shadowNumero()}),
		bloqueConSombras("operation_sub", {A: shadowNumero(), B: shadowNumero()}),
		bloqueConSombras("operation_mul", {A: shadowNumero(), B: shadowNumero()}),
		bloqueConSombras("operation_div", {A: shadowNumero(), B: shadowNumero()}),
	],
	sensor: [
		{kind: "block", type: "sensor_tiempo"},
		{kind: "block", type: "sensor_boton_control_1"},
		{kind: "block", type: "sensor_boton_control_2"},
		{kind: "block", type: "sensor_boton_control_1_presionado"},
		{kind: "block", type: "sensor_boton_control_2_presionado"},
	],
};

export function flyoutDeCategoria(cat, entidad, tipo) {
	if (cat !== "variable") {
		return filtrarFlyout(FLYOUT[cat] ?? [{kind: "label", text: "sin bloques"}], tipo);
	}

	const contents = [{kind: "button", text: "Crear variable", callbackkey: "crearVariable"}];
	const vars = entidad?.Variables ?? [];
	const max = entidad && "GraficosPrincipales" in entidad ? 128 : 43;
	if (vars.length >= max) {
		contents[0] = {kind: "label", text: "Crear variable (maximo)"};
	}

	if (!vars.length) {
		contents.push({kind: "label", text: "sin variables"});
		return contents;
	}

	for (const nombre of vars) {
		const usada = variableEnUso(entidad, nombre);
		contents.push({kind: "label", text: nombre});
		if (usada) {
			contents.push({kind: "label", text: "Eliminar (en uso)"});
		} else {
			contents.push({kind: "button", text: "Eliminar", callbackkey: `eliminarVariable:${nombre}`});
		}
	}

	contents.push({kind: "sep", gap: "12"});
	contents.push({kind: "block", type: "variable"});
	contents.push(bloqueConSombras("variable_store", {VAL: shadowNumero()}));
	return contents;
}

export const CATEGORIAS = ["mocion", "animacion", "capa", "sonido", "control", "evento", "variable", "operacion", "sensor"];
