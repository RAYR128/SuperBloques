import {
	BLOB_SIZES,
	ESCENA_VARIABLES_MAX,
	OBJETO_VARIABLES_MAX,
	zerosBase64,
} from "./project.js";

const OPERACIONES = new Set([
	"numero",
	"motion",
	"motion_get_posicion_x",
	"motion_get_posicion_y",
	"motion_set_posicion_x",
	"motion_set_posicion_y",
	"motion_add_posicion_x",
	"motion_add_posicion_y",
	"animation",
	"sound",
	"control",
	"control_if",
	"control_while",
	"control_ifelse",
	"evento",
	"evento_init",
	"evento_frame",
	"variable",
	"variable_store",
	"operation",
	"operation_add",
	"operation_sub",
	"operation_mul",
	"operation_div",
	"sensor",
	"sensor_tiempo",
	"sensor_boton_control_1",
	"sensor_boton_control_2",
	"sensor_boton_control_1_presionado",
	"sensor_boton_control_2_presionado",
]);

function valorBase64(c) {
	if (c >= 65 && c <= 90) {
		return c - 65;
	}
	if (c >= 97 && c <= 122) {
		return c - 97 + 26;
	}
	if (c >= 48 && c <= 57) {
		return c - 48 + 52;
	}
	if (c === 43) {
		return 62;
	}
	if (c === 47) {
		return 63;
	}
	return -1;
}

function decodificarBase64(entrada, tamanoEsperado) {
	let limpia = "";
	for (let i = 0; i < entrada.length; i++) {
		const c = entrada[i];
		if (c === " " || c === "\n" || c === "\r" || c === "\t") {
			continue;
		}
		limpia += c;
	}
	if (limpia.length % 4 !== 0) {
		return false;
	}

	let escrito = 0;
	for (let i = 0; i < limpia.length; i += 4) {
		let pad = 0;
		const v = [0, 0, 0, 0];
		for (let k = 0; k < 4; k++) {
			const c = limpia[i + k];
			if (c === "=") {
				if (i + 4 !== limpia.length || k < 2) {
					return false;
				}
				v[k] = 0;
				pad++;
			} else {
				if (pad !== 0) {
					return false;
				}
				v[k] = valorBase64(c.charCodeAt(0));
				if (v[k] < 0) {
					return false;
				}
			}
		}
		if (pad > 2) {
			return false;
		}
		const bytes = 3 - pad;
		for (let b = 0; b < bytes; b++) {
			if (escrito >= tamanoEsperado) {
				return false;
			}
			escrito++;
		}
	}
	return escrito === tamanoEsperado;
}

function esObjeto(v) {
	return v !== null && typeof v === "object" && !Array.isArray(v);
}

function objetoOVacio(raiz, clave) {
	if (!Object.prototype.hasOwnProperty.call(raiz, clave)) {
		return {};
	}
	if (!esObjeto(raiz[clave])) {
		throw new Error(clave + " debe ser un objeto");
	}
	return raiz[clave];
}

function resolverParametroEspecial(nodo, variables) {
	if (!Object.prototype.hasOwnProperty.call(nodo, "ParametroEspecial")) {
		return 0;
	}
	const parametro = nodo.ParametroEspecial;
	if (typeof parametro === "number") {
		if (!Number.isInteger(parametro)) {
			throw new Error("ParametroEspecial debe ser entero o string");
		}
		return parametro;
	}
	if (typeof parametro === "string") {
		if (!variables.includes(parametro)) {
			throw new Error("variable inexistente: " + parametro);
		}
		return parametro;
	}
	throw new Error("ParametroEspecial debe ser entero o string");
}

function parsearBloqueAnidado(nodo, variables) {
	if (!esObjeto(nodo)) {
		throw new Error("bloque debe ser un objeto");
	}
	if (typeof nodo.Operacion !== "string") {
		throw new Error("bloque sin Operacion");
	}
	if (!OPERACIONES.has(nodo.Operacion)) {
		throw new Error("Operacion desconocida: " + nodo.Operacion);
	}

	const bloque = {
		Operacion: nodo.Operacion,
		ParametroEspecial: resolverParametroEspecial(nodo, variables),
		Entradas: [],
	};

	if (Array.isArray(nodo.PosicionVisual) && nodo.PosicionVisual.length >= 2) {
		const x = nodo.PosicionVisual[0];
		const y = nodo.PosicionVisual[1];
		if (!Number.isInteger(x) || !Number.isInteger(y)) {
			throw new Error("PosicionVisual debe ser [x, y]");
		}
		bloque.PosicionVisual = [x, y];
	}

	if (Object.prototype.hasOwnProperty.call(nodo, "Entradas")) {
		if (!Array.isArray(nodo.Entradas)) {
			throw new Error("Entradas debe ser un array");
		}
		for (const entrada of nodo.Entradas) {
			bloque.Entradas.push(parsearBloqueAnidado(entrada, variables));
		}
	}
	return bloque;
}

function resolverEnlace(nodo, campo, ids) {
	if (!Object.prototype.hasOwnProperty.call(nodo, campo) || nodo[campo] === null) {
		return null;
	}
	if (typeof nodo[campo] !== "string") {
		throw new Error(campo + " debe ser string o null");
	}
	const id = nodo[campo];
	if (!ids.has(id)) {
		throw new Error("id de bloque inexistente en " + campo + ": " + id);
	}
	return id;
}

function parsearBloques(nodoBloques, variables) {
	if (!esObjeto(nodoBloques)) {
		throw new Error("Bloques debe ser un objeto");
	}

	const ids = new Set(Object.keys(nodoBloques));
	const bloques = {};
	for (const id of ids) {
		bloques[id] = parsearBloqueAnidado(nodoBloques[id], variables);
		bloques[id].Siguiente = null;
		bloques[id].Previo = null;
	}

	for (const id of ids) {
		const origen = nodoBloques[id];
		const destino = bloques[id];
		destino.Siguiente = resolverEnlace(origen, "Siguiente", ids);
		destino.Previo = resolverEnlace(origen, "Previo", ids);
		const cuerpo = resolverEnlace(origen, "Cuerpo", ids);
		if (cuerpo !== null) {
			destino.Cuerpo = cuerpo;
		}
		const cuerpoSino = resolverEnlace(origen, "CuerpoSino", ids);
		if (cuerpoSino !== null) {
			destino.CuerpoSino = cuerpoSino;
		}
	}
	return bloques;
}

function parsearBloquesOpcional(entidad, variables) {
	if (!Object.prototype.hasOwnProperty.call(entidad, "Bloques")) {
		return {};
	}
	return parsearBloques(entidad.Bloques, variables);
}

function parsearBlob(nodo, clave, tamano) {
	if (!Object.prototype.hasOwnProperty.call(nodo, clave)) {
		return zerosBase64(tamano);
	}
	if (typeof nodo[clave] !== "string") {
		throw new Error(clave + " debe ser string base64");
	}
	if (!decodificarBase64(nodo[clave], tamano)) {
		throw new Error("base64 invalido o tamano incorrecto: " + clave);
	}
	return nodo[clave];
}

function parsearVariables(nodo, maximo) {
	if (!Object.prototype.hasOwnProperty.call(nodo, "Variables")) {
		return [];
	}
	if (!Array.isArray(nodo.Variables)) {
		throw new Error("Variables debe ser un array");
	}
	if (nodo.Variables.length > maximo) {
		throw new Error("demasiadas variables (max " + maximo + ")");
	}

	const vistos = new Set();
	const salida = [];
	for (const item of nodo.Variables) {
		if (typeof item !== "string") {
			throw new Error("variable debe ser un string");
		}
		if (item === "") {
			throw new Error("nombre de variable vacio");
		}
		if (vistos.has(item)) {
			throw new Error("variable duplicada: " + item);
		}
		vistos.add(item);
		salida.push(item);
	}
	return salida;
}

export function parsearProyecto(texto) {
	if (typeof texto === "string" && texto.charCodeAt(0) === 0xfeff) {
		texto = texto.slice(1);
	}

	let raiz;
	try {
		raiz = JSON.parse(texto);
	} catch {
		throw new Error("JSON malformado");
	}
	if (!esObjeto(raiz)) {
		throw new Error("el proyecto debe ser un objeto");
	}

	const escenasJson = objetoOVacio(raiz, "Escenas");
	const objetosJson = objetoOVacio(raiz, "Objetos");
	const escenas = {};
	const objetos = {};

	for (const [nombre, valor] of Object.entries(escenasJson)) {
		if (!esObjeto(valor)) {
			throw new Error("escena debe ser un objeto");
		}
		const variables = parsearVariables(valor, ESCENA_VARIABLES_MAX);
		escenas[nombre] = {
			GraficosPrincipales: parsearBlob(valor, "GraficosPrincipales", BLOB_SIZES.GraficosPrincipales),
			GraficosHud: parsearBlob(valor, "GraficosHud", BLOB_SIZES.GraficosHud),
			Tilemap1: parsearBlob(valor, "Tilemap1", BLOB_SIZES.Tilemap1),
			Tilemap2: parsearBlob(valor, "Tilemap2", BLOB_SIZES.Tilemap2),
			Tilemap3: parsearBlob(valor, "Tilemap3", BLOB_SIZES.Tilemap3),
			Paleta: parsearBlob(valor, "Paleta", BLOB_SIZES.Paleta),
			Variables: variables,
			Bloques: parsearBloquesOpcional(valor, variables),
		};
	}

	for (const [nombre, valor] of Object.entries(objetosJson)) {
		if (!esObjeto(valor)) {
			throw new Error("objeto debe ser un objeto");
		}
		const variables = parsearVariables(valor, OBJETO_VARIABLES_MAX);
		objetos[nombre] = {
			Variables: variables,
			Bloques: parsearBloquesOpcional(valor, variables),
		};
	}

	return {Escenas: escenas, Objetos: objetos};
}
