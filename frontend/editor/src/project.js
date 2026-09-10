import graficosHudDefectoUrl from "./defaults/GraficosHudDefecto.bin?inline";
import paletaDefectoUrl from "./defaults/PaletaColoresDefecto.bin?inline";

export const BLOB_SIZES = {
	GraficosPrincipales: 32768,
	GraficosHud: 4096,
	Tilemap1: 8192,
	Tilemap2: 2048,
	Tilemap3: 2048,
	Paleta: 512,
};

function base64DeDataUrl(dataUrl) {
	const i = dataUrl.indexOf(",");
	return i >= 0 ? dataUrl.slice(i + 1) : dataUrl;
}

const GRAFICOS_HUD_DEFECTO = base64DeDataUrl(graficosHudDefectoUrl);
const PALETA_DEFECTO = base64DeDataUrl(paletaDefectoUrl);

export const ESCENA_VARIABLES_MAX = 128;
export const OBJETO_VARIABLES_MAX = 43;

const listeners = new Set();

export const NOMBRE_ARCHIVO_DEFECTO = "Nuevo Proyecto.json";

export const state = {
	proyecto: proyectoPorDefecto(),
	seleccion: {tipo: "escena", nombre: "Escena1"},
	nombreArchivo: NOMBRE_ARCHIVO_DEFECTO,
	generation: 0,
};

export function subscribe(fn) {
	listeners.add(fn);
	return () => listeners.delete(fn);
}

export function notify() {
	for (const fn of listeners) {
		fn();
	}
}

export function zerosBase64(n) {
	const bytes = new Uint8Array(n);
	const chunk = 0x8000;
	let binary = "";
	for (let i = 0; i < bytes.length; i += chunk) {
		binary += String.fromCharCode.apply(null, bytes.subarray(i, i + chunk));
	}
	return btoa(binary);
}

export function crearEscena() {
	return {
		GraficosPrincipales: zerosBase64(BLOB_SIZES.GraficosPrincipales),
		GraficosHud: GRAFICOS_HUD_DEFECTO,
		Tilemap1: zerosBase64(BLOB_SIZES.Tilemap1),
		Tilemap2: zerosBase64(BLOB_SIZES.Tilemap2),
		Tilemap3: zerosBase64(BLOB_SIZES.Tilemap3),
		Paleta: PALETA_DEFECTO,
		Variables: [],
		Bloques: {},
	};
}

export function crearObjeto() {
	return {
		Variables: [],
		Bloques: {},
	};
}

export function proyectoPorDefecto() {
	return {
		Escenas: {Escena1: crearEscena()},
		Objetos: {},
	};
}

export function mapaDe(tipo) {
	return tipo === "escena" ? state.proyecto.Escenas : state.proyecto.Objetos;
}

export function entidadActual() {
	if (!state.seleccion) {
		return null;
	}
	return mapaDe(state.seleccion.tipo)[state.seleccion.nombre] ?? null;
}

export function variablesActuales() {
	return entidadActual()?.Variables ?? [];
}

export function maxVariables() {
	if (!state.seleccion) {
		return 0;
	}
	return state.seleccion.tipo === "escena" ? ESCENA_VARIABLES_MAX : OBJETO_VARIABLES_MAX;
}

export function pedirNombre(titulo, ocupados) {
	const raw = window.prompt(titulo);
	if (raw === null) {
		return null;
	}
	const nombre = raw.trim();
	if (!nombre) {
		window.alert("El nombre no puede estar vacio");
		return null;
	}
	if (ocupados.has(nombre)) {
		window.alert("Ya existe un elemento con ese nombre");
		return null;
	}
	return nombre;
}

export function agregarEscena() {
	const nombre = pedirNombre("Nombre de la escena", new Set(Object.keys(state.proyecto.Escenas)));
	if (!nombre) {
		return null;
	}
	state.proyecto.Escenas[nombre] = crearEscena();
	state.seleccion = {tipo: "escena", nombre};
	notify();
	return nombre;
}

export function agregarObjeto() {
	const nombre = pedirNombre("Nombre del objeto", new Set(Object.keys(state.proyecto.Objetos)));
	if (!nombre) {
		return null;
	}
	state.proyecto.Objetos[nombre] = crearObjeto();
	state.seleccion = {tipo: "objeto", nombre};
	notify();
	return nombre;
}

function siguienteSeleccion(tipo, nombre) {
	const propio = Object.keys(mapaDe(tipo)).filter((n) => n !== nombre);
	if (propio.length) {
		return {tipo, nombre: propio[0]};
	}
	const otroTipo = tipo === "escena" ? "objeto" : "escena";
	const otro = Object.keys(mapaDe(otroTipo));
	if (otro.length) {
		return {tipo: otroTipo, nombre: otro[0]};
	}
	return null;
}

export function eliminarEntidad(tipo, nombre) {
	const etiqueta = tipo === "escena" ? "escena" : "objeto";
	if (tipo === "escena" && escenaEnUso(nombre)) {
		window.alert(`No se puede eliminar la escena "${nombre}" porque esta en uso`);
		return false;
	}
	if (!window.confirm(`Eliminar ${etiqueta} "${nombre}"?`)) {
		return false;
	}
	delete mapaDe(tipo)[nombre];
	if (state.seleccion && state.seleccion.tipo === tipo && state.seleccion.nombre === nombre) {
		state.seleccion = siguienteSeleccion(tipo, nombre);
	}
	notify();
	return true;
}

export function seleccionar(tipo, nombre) {
	if (state.seleccion && state.seleccion.tipo === tipo && state.seleccion.nombre === nombre) {
		return;
	}
	state.seleccion = {tipo, nombre};
	notify();
}

export function agregarVariable() {
	const entidad = entidadActual();
	if (!entidad) {
		return null;
	}
	if (entidad.Variables.length >= maxVariables()) {
		window.alert(`Maximo de ${maxVariables()} variables`);
		return null;
	}
	const nombre = pedirNombre("Nombre de la variable", new Set(entidad.Variables));
	if (!nombre) {
		return null;
	}
	entidad.Variables = [...entidad.Variables, nombre];
	notify();
	return nombre;
}

export function eliminarVariable(nombre) {
	const entidad = entidadActual();
	if (!entidad) {
		return false;
	}
	if (variableEnUso(entidad, nombre)) {
		return false;
	}
	entidad.Variables = entidad.Variables.filter((n) => n !== nombre);
	notify();
	return true;
}

function visitaNodo(nodo, nombre) {
	if (!nodo) {
		return false;
	}
	if ((nodo.Operacion === "variable" || nodo.Operacion === "variable_store") && nodo.ParametroEspecial === nombre) {
		return true;
	}
	for (const entrada of nodo.Entradas || []) {
		if (visitaNodo(entrada, nombre)) {
			return true;
		}
	}
	return false;
}

export function variableEnUso(entidad, nombre) {
	if (!entidad) {
		return false;
	}
	for (const bloque of Object.values(entidad.Bloques || {})) {
		if (visitaNodo(bloque, nombre)) {
			return true;
		}
	}
	return false;
}

function visitaNodoEscena(nodo, nombre) {
	if (!nodo) {
		return false;
	}
	if (nodo.Operacion === "evento_cambiar_escena" && nodo.ParametroEspecial === nombre) {
		return true;
	}
	for (const entrada of nodo.Entradas || []) {
		if (visitaNodoEscena(entrada, nombre)) {
			return true;
		}
	}
	return false;
}

export function escenaEnUso(nombre) {
	const entidades = [...Object.values(state.proyecto.Escenas), ...Object.values(state.proyecto.Objetos)];
	for (const entidad of entidades) {
		for (const bloque of Object.values(entidad.Bloques || {})) {
			if (visitaNodoEscena(bloque, nombre)) {
				return true;
			}
		}
	}
	return false;
}

export function guardarBloquesActuales(bloques) {
	const entidad = entidadActual();
	if (!entidad) {
		return;
	}
	entidad.Bloques = bloques;
}

function primeraSeleccion(proyecto) {
	const escenas = Object.keys(proyecto.Escenas);
	if (escenas.length) {
		return {tipo: "escena", nombre: escenas[0]};
	}
	const objetos = Object.keys(proyecto.Objetos);
	if (objetos.length) {
		return {tipo: "objeto", nombre: objetos[0]};
	}
	return null;
}

function tituloDeArchivo(nombreArchivo) {
	const nombre = (nombreArchivo || NOMBRE_ARCHIVO_DEFECTO).replace(/\.json$/i, "").trim();
	return nombre || "Nuevo Proyecto";
}

function actualizarTitulo() {
	const el = document.querySelector("header .sub");
	if (el) {
		el.textContent = tituloDeArchivo(state.nombreArchivo);
	}
}

export function cargarProyecto(proyecto, nombreArchivo) {
	state.proyecto = proyecto;
	state.nombreArchivo = nombreArchivo || NOMBRE_ARCHIVO_DEFECTO;
	state.generation += 1;
	state.seleccion = primeraSeleccion(proyecto);
	actualizarTitulo();
	notify();
}
