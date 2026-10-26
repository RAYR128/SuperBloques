import {CANALES, FRECUENCIA_C4, NOTA_MAX} from "./musica/notas.js";
import {
	BLOB_SIZES,
	ESCENA_VARIABLES_MAX,
	OBJETO_VARIABLES_MAX,
	zerosBase64,
} from "./project.js";
import {BRR_DATOS_MAX, SONIDOS_MAX} from "./sonido/brr.js";

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
	"animacion_obj_set_sprite",
	"animacion_scene_set_mosaic_filter",
	"animacion_scene_set_brightness",
	"animacion_scene_set_color",
	"capa",
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
	"sound",
	"control",
	"control_if",
	"control_while",
	"control_ifelse",
	"evento",
	"evento_init",
	"evento_frame",
	"evento_estado",
	"evento_set_estado",
	"evento_cambiar_escena",
	"variable",
	"variable_store",
	"operation",
	"operation_add",
	"operation_sub",
	"operation_mul",
	"operation_div",
	"operation_mod",
	"operation_and",
	"operation_or",
	"operation_xor",
	"operation_asl",
	"operation_lsr",
	"operation_rol",
	"operation_ror",
	"operation_eq",
	"operation_neq",
	"operation_gt",
	"operation_gte",
	"operation_lt",
	"operation_lte",
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

function longitudBase64(entrada) {
	let limpia = "";
	for (let i = 0; i < entrada.length; i++) {
		const c = entrada[i];
		if (c === " " || c === "\n" || c === "\r" || c === "\t") {
			continue;
		}
		limpia += c;
	}
	if (limpia.length === 0 || limpia.length % 4 !== 0) {
		return -1;
	}

	let escrito = 0;
	for (let i = 0; i < limpia.length; i += 4) {
		let pad = 0;
		for (let k = 0; k < 4; k++) {
			const c = limpia[i + k];
			if (c === "=") {
				if (i + 4 !== limpia.length || k < 2) {
					return -1;
				}
				pad++;
			} else {
				if (pad !== 0 || valorBase64(c.charCodeAt(0)) < 0) {
					return -1;
				}
			}
		}
		if (pad > 2) {
			return -1;
		}
		escrito += 3 - pad;
	}
	return escrito;
}

function decodificarBase64(entrada, tamanoEsperado) {
	return longitudBase64(entrada) === tamanoEsperado;
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
		if (nodo.Operacion === "evento_cambiar_escena") {
			return parametro;
		}
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

function visitaNodo(nodo, fn) {
	if (!nodo) {
		return;
	}
	fn(nodo);
	for (const entrada of nodo.Entradas || []) {
		visitaNodo(entrada, fn);
	}
}

function validarEscenasReferenciadas(proyecto) {
	const nombres = new Set(Object.keys(proyecto.Escenas));
	const revisar = (bloques) => {
		for (const bloque of Object.values(bloques || {})) {
			visitaNodo(bloque, (nodo) => {
				if (nodo.Operacion !== "evento_cambiar_escena") {
					return;
				}
				const parametro = nodo.ParametroEspecial;
				if (typeof parametro === "string" && !nombres.has(parametro)) {
					throw new Error("escena inexistente: " + parametro);
				}
			});
		}
	};
	for (const escena of Object.values(proyecto.Escenas)) {
		revisar(escena.Bloques);
	}
	for (const objeto of Object.values(proyecto.Objetos)) {
		revisar(objeto.Bloques);
	}
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

function enteroEnRango(nodo, clave, min, max, defecto) {
	if (!Object.prototype.hasOwnProperty.call(nodo, clave)) {
		return defecto;
	}
	const valor = nodo[clave];
	if (!Number.isInteger(valor) || valor < min || valor > max) {
		throw new Error(clave + " fuera de rango");
	}
	return valor;
}

function boolOpcional(nodo, clave, defecto) {
	if (!Object.prototype.hasOwnProperty.call(nodo, clave)) {
		return defecto;
	}
	if (typeof nodo[clave] !== "boolean") {
		throw new Error(clave + " debe ser bool");
	}
	return nodo[clave];
}

function parsearSonidos(sonidosJson) {
	const nombres = Object.keys(sonidosJson);
	if (nombres.length > SONIDOS_MAX) {
		throw new Error("demasiados sonidos (max " + SONIDOS_MAX + ")");
	}
	const sonidos = {};
	for (const nombre of nombres) {
		if (nombre === "") {
			throw new Error("nombre de sonido vacio");
		}
		const valor = sonidosJson[nombre];
		if (!esObjeto(valor)) {
			throw new Error("sonido debe ser un objeto");
		}
		if (typeof valor.Datos !== "string") {
			throw new Error("Datos debe ser string base64");
		}
		const longitud = longitudBase64(valor.Datos);
		if (longitud < 0) {
			throw new Error("base64 invalido: Datos");
		}
		if (longitud === 0 || longitud % 9 !== 0 || longitud > BRR_DATOS_MAX) {
			throw new Error("Datos BRR invalidos: " + nombre);
		}
		if (!Number.isInteger(valor.PuntoLoop)) {
			throw new Error("PuntoLoop debe ser entero");
		}
		if (valor.PuntoLoop < 0 || valor.PuntoLoop >= longitud || valor.PuntoLoop % 9 !== 0) {
			throw new Error("PuntoLoop invalido: " + nombre);
		}
		sonidos[nombre] = {Datos: valor.Datos, PuntoLoop: valor.PuntoLoop};
	}
	return sonidos;
}

function parsearInstrumento(nombre, valor, sonidos) {
	if (!esObjeto(valor)) {
		throw new Error("instrumento debe ser un objeto");
	}
	if (typeof valor.Sonido !== "string" || !Object.prototype.hasOwnProperty.call(sonidos, valor.Sonido)) {
		throw new Error("sonido inexistente: " + valor.Sonido);
	}
	return {
		Sonido: valor.Sonido,
		Frecuencia: enteroEnRango(valor, "Frecuencia", 0, 65535, FRECUENCIA_C4),
		EsGain: boolOpcional(valor, "EsGain", false),
		A: enteroEnRango(valor, "A", 0, 15, 0),
		D: enteroEnRango(valor, "D", 0, 7, 0),
		S: enteroEnRango(valor, "S", 0, 7, 0),
		R: enteroEnRango(valor, "R", 0, 31, 0),
		Gain: enteroEnRango(valor, "Gain", 0, 127, 127),
	};
}

function parsearNota(valor, instrumentos) {
	if (!esObjeto(valor)) {
		throw new Error("nota debe ser un objeto");
	}
	if (typeof valor.Instrumento !== "string" || !instrumentos.has(valor.Instrumento)) {
		throw new Error("instrumento inexistente: " + valor.Instrumento);
	}
	if (!Number.isInteger(valor.Nota) || valor.Nota < 0 || valor.Nota > NOTA_MAX) {
		throw new Error("Nota fuera de rango");
	}
	if (!Number.isInteger(valor.Posicion) || valor.Posicion < 0) {
		throw new Error("Posicion fuera de rango");
	}
	if (!Number.isInteger(valor.Duracion) || valor.Duracion < 1) {
		throw new Error("Duracion fuera de rango");
	}
	return {
		Instrumento: valor.Instrumento,
		Nota: valor.Nota,
		Posicion: valor.Posicion,
		Duracion: valor.Duracion,
	};
}

function parsearMusica(musicaJson, sonidos) {
	const musica = {};
	for (const [nombre, valor] of Object.entries(musicaJson)) {
		if (nombre === "") {
			throw new Error("nombre de musica vacio");
		}
		if (!esObjeto(valor)) {
			throw new Error("musica debe ser un objeto");
		}
		const instrumentosJson = objetoOVacio(valor, "Instrumentos");
		const instrumentos = {};
		for (const [instNombre, instValor] of Object.entries(instrumentosJson)) {
			if (instNombre === "") {
				throw new Error("nombre de instrumento vacio");
			}
			instrumentos[instNombre] = parsearInstrumento(instNombre, instValor, sonidos);
		}
		const notasJson = objetoOVacio(valor, "Notas");
		const notas = {};
		const nombresInst = new Set(Object.keys(instrumentos));
		for (const [canal, lista] of Object.entries(notasJson)) {
			if (!CANALES.includes(canal)) {
				throw new Error("canal desconocido: " + canal);
			}
			if (!Array.isArray(lista)) {
				throw new Error(canal + " debe ser un array");
			}
			notas[canal] = lista.map((nota) => parsearNota(nota, nombresInst));
		}
		musica[nombre] = {Instrumentos: instrumentos, Notas: notas};
	}
	return musica;
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

	const sonidos = parsearSonidos(objetoOVacio(raiz, "Sonidos"));
	const musica = parsearMusica(objetoOVacio(raiz, "Musica"), sonidos);
	const proyecto = {Escenas: escenas, Objetos: objetos, Sonidos: sonidos, Musica: musica};
	validarEscenasReferenciadas(proyecto);
	return proyecto;
}
