import graficosHudDefectoUrl from "./defaults/GraficosHudDefecto.bin?inline";
import paletaDefectoUrl from "./defaults/PaletaColoresDefecto.bin?inline";
import {FRECUENCIA_C4} from "./musica/notas.js";
import {SONIDOS_MAX} from "./sonido/brr.js";

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
	projectId: null,
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

export function crearInstrumento(sonido) {
	return {
		Sonido: sonido,
		Frecuencia: FRECUENCIA_C4,
		EsGain: false,
		A: 0,
		D: 0,
		S: 0,
		R: 0,
		Gain: 127,
	};
}

export function crearMusica() {
	return {
		Instrumentos: {},
		Notas: {},
	};
}

export function proyectoPorDefecto() {
	return {
		Escenas: {Escena1: crearEscena()},
		Objetos: {},
		Sonidos: {},
		Musica: {},
	};
}

export function esEditorDeBloques(tipo) {
	return tipo === "escena" || tipo === "objeto";
}

export function mapaDe(tipo) {
	if (tipo === "escena") {
		return state.proyecto.Escenas;
	}
	if (tipo === "objeto") {
		return state.proyecto.Objetos;
	}
	if (tipo === "sonido") {
		return state.proyecto.Sonidos;
	}
	if (tipo === "musica") {
		return state.proyecto.Musica;
	}
	return {};
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

export function incorporarSonido(nombreInicial, datos, puntoLoop) {
	if (Object.keys(state.proyecto.Sonidos).length >= SONIDOS_MAX) {
		window.alert(`Maximo de ${SONIDOS_MAX} sonidos`);
		return null;
	}
	const ocupados = new Set(Object.keys(state.proyecto.Sonidos));
	let nombre = (nombreInicial || "").trim();
	if (!nombre || ocupados.has(nombre)) {
		nombre = pedirNombre("Nombre del sonido", ocupados);
		if (!nombre) {
			return null;
		}
	}
	state.proyecto.Sonidos[nombre] = {Datos: datos, PuntoLoop: puntoLoop};
	state.seleccion = {tipo: "sonido", nombre};
	notify();
	return nombre;
}

export function agregarMusica() {
	const nombre = pedirNombre("Nombre de la musica", new Set(Object.keys(state.proyecto.Musica)));
	if (!nombre) {
		return null;
	}
	state.proyecto.Musica[nombre] = crearMusica();
	state.seleccion = {tipo: "musica", nombre};
	notify();
	return nombre;
}

export function agregarInstrumento(musicaNombre) {
	const musica = state.proyecto.Musica[musicaNombre];
	if (!musica) {
		return null;
	}
	const sonidos = Object.keys(state.proyecto.Sonidos);
	if (!sonidos.length) {
		window.alert("Agrega un sonido antes de crear un instrumento");
		return null;
	}
	const nombre = pedirNombre("Nombre del instrumento", new Set(Object.keys(musica.Instrumentos)));
	if (!nombre) {
		return null;
	}
	musica.Instrumentos[nombre] = crearInstrumento(sonidos[0]);
	notify();
	return nombre;
}

export function eliminarInstrumento(musicaNombre, nombre) {
	const musica = state.proyecto.Musica[musicaNombre];
	if (!musica || !musica.Instrumentos[nombre]) {
		return false;
	}
	if (instrumentoEnUso(musicaNombre, nombre)) {
		window.alert(`No se puede eliminar el instrumento "${nombre}" porque esta en uso`);
		return false;
	}
	if (!window.confirm(`Eliminar instrumento "${nombre}"?`)) {
		return false;
	}
	delete musica.Instrumentos[nombre];
	notify();
	return true;
}

export function insertarNota(musicaNombre, canal, nota) {
	const musica = state.proyecto.Musica[musicaNombre];
	if (!musica) {
		return -1;
	}
	if (!musica.Notas[canal]) {
		musica.Notas[canal] = [];
	}
	musica.Notas[canal].push(nota);
	notify();
	return musica.Notas[canal].length - 1;
}

export function cambiarDuracionNota(musicaNombre, canal, indice, duracion) {
	const nota = state.proyecto.Musica[musicaNombre]?.Notas?.[canal]?.[indice];
	if (!nota) {
		return;
	}
	nota.Duracion = duracion;
	notify();
}

export function eliminarNota(musicaNombre, canal, indice) {
	const lista = state.proyecto.Musica[musicaNombre]?.Notas?.[canal];
	if (!lista || indice < 0 || indice >= lista.length) {
		return false;
	}
	lista.splice(indice, 1);
	notify();
	return true;
}

const ORDEN_LISTAS = ["escena", "objeto", "sonido", "musica"];

function siguienteSeleccion(tipo, nombre) {
	const propio = Object.keys(mapaDe(tipo)).filter((n) => n !== nombre);
	if (propio.length) {
		return {tipo, nombre: propio[0]};
	}
	for (const otroTipo of ORDEN_LISTAS) {
		if (otroTipo === tipo) {
			continue;
		}
		const otro = Object.keys(mapaDe(otroTipo));
		if (otro.length) {
			return {tipo: otroTipo, nombre: otro[0]};
		}
	}
	return null;
}

const ETIQUETAS = {
	escena: "escena",
	objeto: "objeto",
	sonido: "sonido",
	musica: "musica",
};

export function eliminarEntidad(tipo, nombre) {
	const etiqueta = ETIQUETAS[tipo] ?? tipo;
	if (tipo === "escena" && escenaEnUso(nombre)) {
		window.alert(`No se puede eliminar la escena "${nombre}" porque esta en uso`);
		return false;
	}
	if (tipo === "sonido" && sonidoEnUso(nombre)) {
		window.alert(`No se puede eliminar el sonido "${nombre}" porque esta en uso`);
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

export function sonidoEnUso(nombre) {
	for (const musica of Object.values(state.proyecto.Musica)) {
		for (const instrumento of Object.values(musica.Instrumentos || {})) {
			if (instrumento.Sonido === nombre) {
				return true;
			}
		}
	}
	return false;
}

export function instrumentoEnUso(musicaNombre, nombre) {
	const musica = state.proyecto.Musica[musicaNombre];
	if (!musica) {
		return false;
	}
	for (const canal of Object.values(musica.Notas || {})) {
		for (const nota of canal) {
			if (nota.Instrumento === nombre) {
				return true;
			}
		}
	}
	return false;
}

export function guardarBloquesActuales(bloques) {
	if (!esEditorDeBloques(state.seleccion?.tipo)) {
		return;
	}
	const entidad = entidadActual();
	if (!entidad) {
		return;
	}
	entidad.Bloques = bloques;
}

function primeraSeleccion(proyecto) {
	const mapas = [
		["escena", proyecto.Escenas],
		["objeto", proyecto.Objetos],
		["musica", proyecto.Musica],
		["sonido", proyecto.Sonidos],
	];
	for (const [tipo, mapa] of mapas) {
		const nombres = Object.keys(mapa || {});
		if (nombres.length) {
			return {tipo, nombre: nombres[0]};
		}
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

export function cargarProyecto(proyecto, nombreArchivo, projectId) {
	state.proyecto = proyecto;
	state.nombreArchivo = nombreArchivo || NOMBRE_ARCHIVO_DEFECTO;
	state.projectId = projectId ?? null;
	state.generation += 1;
	state.seleccion = primeraSeleccion(proyecto);
	actualizarTitulo();
	notify();
}

export function marcarPublicado(id, nombre) {
	state.projectId = id;
	if (nombre) {
		state.nombreArchivo = String(nombre).replace(/\.json$/i, "") + ".json";
	}
	actualizarTitulo();
}

export function tituloProyecto() {
	return tituloDeArchivo(state.nombreArchivo);
}
