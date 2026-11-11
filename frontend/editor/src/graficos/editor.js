import {state, subscribe} from "../project.js";
import {circulo, circuloRelleno, linea, rectangulo, rectanguloRelleno} from "./formas.js";
import {
	GFX,
	bytesABase64,
	bytesDeBase64,
	canal8,
	color15,
	escribirColor15,
	escribirIndice,
	leerIndice,
	offsetColor,
	renderHoja,
	rgbCss,
} from "./tiles.js";

const ZOOMS = [1, 2, 4, 8, 16];
const HISTORIA_MAX = 100;

let vista = null;
let clave = "";
let paletaIndex = 0;
let colorIndex = 1;
let zoom = 4;
let grilla = 8;
let herramienta = "lapiz";
let gesto = null;
let cursorPixel = null;
let deshacerPila = [];
let rehacerPila = [];

function $(id) {
	return document.getElementById(id);
}

function claveDe(sel) {
	if (!sel || sel.tipo !== "gfx") {
		return "";
	}
	return `${state.generation}:${sel.gfx}:${sel.nombre}`;
}

function generacionDeClave(valor) {
	if (!valor) {
		return null;
	}
	const corte = valor.indexOf(":");
	if (corte < 0) {
		return null;
	}
	const n = Number(valor.slice(0, corte));
	return Number.isInteger(n) ? n : null;
}

function crearVista(sel) {
	if (sel.gfx === "objetos") {
		return {
			gfx: "objetos",
			leerGfx: () => state.proyecto.GraficosObjetos,
			escribirGfx: (b64) => {
				state.proyecto.GraficosObjetos = b64;
			},
			leerPaleta: () => state.proyecto.PaletaObjetos,
			escribirPaleta: (b64) => {
				state.proyecto.PaletaObjetos = b64;
			},
		};
	}
	const escena = state.proyecto.Escenas[sel.nombre];
	const campo = sel.gfx === "hud" ? "GraficosHud" : "GraficosPrincipales";
	return {
		gfx: sel.gfx,
		leerGfx: () => escena[campo],
		escribirGfx: (b64) => {
			escena[campo] = b64;
		},
		leerPaleta: () => escena.Paleta,
		escribirPaleta: (b64) => {
			escena.Paleta = b64;
		},
	};
}

function decodificar(b64, tamano) {
	const bytes = bytesDeBase64(b64);
	if (bytes.length !== tamano) {
		throw new Error("blob de graficos con tamano incorrecto");
	}
	return bytes;
}

function copiarGfx() {
	return vista.gfxBytes.slice();
}

function bytesIguales(a, b) {
	if (a.length !== b.length) {
		return false;
	}
	for (let i = 0; i < a.length; i++) {
		if (a[i] !== b[i]) {
			return false;
		}
	}
	return true;
}

function actualizarHistoria() {
	const bloqueado = !vista || gesto !== null;
	$("gfx-undo").disabled = bloqueado || deshacerPila.length === 0;
	$("gfx-redo").disabled = bloqueado || rehacerPila.length === 0;
}

function olvidarHistoria() {
	deshacerPila = [];
	rehacerPila = [];
	actualizarHistoria();
}

function comprometer() {
	vista.escribirGfx(bytesABase64(vista.gfxBytes));
}

function aplicarBytes(bytes) {
	vista.gfxBytes = bytes;
	comprometer();
	redibujarHoja();
}

function deshacer() {
	if (!vista || gesto || deshacerPila.length === 0) {
		return;
	}
	const actual = copiarGfx();
	aplicarBytes(deshacerPila.pop());
	rehacerPila.push(actual);
	actualizarHistoria();
}

function rehacer() {
	if (!vista || gesto || rehacerPila.length === 0) {
		return;
	}
	const actual = copiarGfx();
	aplicarBytes(rehacerPila.pop());
	deshacerPila.push(actual);
	actualizarHistoria();
}

function montar(sel) {
	const fuente = crearVista(sel);
	const spec = GFX[fuente.gfx];
	vista = fuente;
	vista.gfxBytes = decodificar(fuente.leerGfx(), spec.bytes);
	vista.paletaBytes = decodificar(fuente.leerPaleta(), 256);
	paletaIndex = 0;
	colorIndex = 1;
	$("gfx-paleta").value = "0";
	const canvas = $("gfx-canvas");
	canvas.width = spec.ancho;
	canvas.height = spec.alto;
	const preview = $("gfx-preview");
	preview.width = spec.ancho;
	preview.height = spec.alto;
	aplicarZoom();
	pintarColores();
	redibujarHoja();
	olvidarCursor();
	actualizarHistoria();
}

function sincronizar() {
	const siguiente = claveDe(state.seleccion);
	if (siguiente === clave) {
		return;
	}
	// Un proyecto reemplazado no debe recibir los bytes de la hoja que se suelta.
	const mismoProyecto = generacionDeClave(clave) === state.generation;
	finalizarGesto(mismoProyecto);
	olvidarHistoria();
	clave = siguiente;
	if (!siguiente) {
		vista = null;
		gesto = null;
		olvidarCursor();
		return;
	}
	montar(state.seleccion);
}

function colorActual() {
	return color15(vista.paletaBytes, offsetColor(vista.gfx, paletaIndex, colorIndex));
}

function pintarColores() {
	const spec = GFX[vista.gfx];
	const cont = $("gfx-colores");
	cont.replaceChildren();
	for (let i = 0; i < spec.colores; i++) {
		const btn = document.createElement("button");
		btn.type = "button";
		btn.className = "palette-color" + (i === colorIndex ? " active" : "");
		const sw = document.createElement("span");
		sw.className = "palette-swatch" + (i === 0 ? " transparente" : "");
		const color = color15(vista.paletaBytes, offsetColor(vista.gfx, paletaIndex, i));
		sw.style.backgroundColor = rgbCss(color.r, color.g, color.b);
		const label = document.createElement("span");
		label.textContent = i === 0 ? "Color 0 (transparente)" : `Color ${i}`;
		btn.append(sw, label);
		btn.addEventListener("click", () => {
			colorIndex = i;
			pintarColores();
		});
		cont.append(btn);
	}
	escribirRgb();
}

function escribirRgb() {
	const color = colorActual();
	for (const [id, valor] of [
		["gfx-r", color.r],
		["gfx-g", color.g],
		["gfx-b", color.b],
	]) {
		const el = $(id);
		if (document.activeElement !== el) {
			el.value = String(valor);
		}
	}
}

function actualizarMuestra() {
	const sw = document.querySelector("#gfx-colores .palette-color.active .palette-swatch");
	if (!sw) {
		return;
	}
	const color = colorActual();
	sw.style.backgroundColor = rgbCss(color.r, color.g, color.b);
}

function canalDe(id) {
	const n = Number($(id).value);
	if (!Number.isInteger(n) || n < 0 || n > 31) {
		return null;
	}
	return n;
}

function aplicarRgb() {
	if (!vista) {
		return;
	}
	const r = canalDe("gfx-r");
	const g = canalDe("gfx-g");
	const b = canalDe("gfx-b");
	if (r === null || g === null || b === null) {
		return;
	}
	escribirColor15(vista.paletaBytes, offsetColor(vista.gfx, paletaIndex, colorIndex), r, g, b);
	vista.escribirPaleta(bytesABase64(vista.paletaBytes));
	actualizarMuestra();
	redibujarHoja();
}

function redibujarHoja() {
	const spec = GFX[vista.gfx];
	const canvas = $("gfx-canvas");
	const ctx = canvas.getContext("2d");
	const img = ctx.createImageData(spec.ancho, spec.alto);
	renderHoja(vista.gfxBytes, vista.paletaBytes, vista.gfx, paletaIndex, img);
	ctx.putImageData(img, 0, 0);
}

function pintarPixel(px, py) {
	const canvas = $("gfx-canvas");
	const ctx = canvas.getContext("2d");
	const img = ctx.createImageData(1, 1);
	const idx = leerIndice(vista.gfxBytes, vista.gfx, px, py);
	if (idx === 0) {
		img.data[3] = 0;
	} else {
		const color = color15(vista.paletaBytes, offsetColor(vista.gfx, paletaIndex, idx));
		img.data[0] = canal8(color.r);
		img.data[1] = canal8(color.g);
		img.data[2] = canal8(color.b);
		img.data[3] = 255;
	}
	ctx.putImageData(img, px, py);
}

function aplicarZoom() {
	const spec = GFX[vista.gfx];
	const w = spec.ancho * zoom;
	const h = spec.alto * zoom;
	const canvas = $("gfx-canvas");
	const preview = $("gfx-preview");
	const grid = $("gfx-grid");
	const stage = $("gfx-stage");
	canvas.style.width = `${w}px`;
	canvas.style.height = `${h}px`;
	preview.style.width = `${w}px`;
	preview.style.height = `${h}px`;
	stage.style.width = `${w}px`;
	stage.style.height = `${h}px`;
	grid.width = w;
	grid.height = h;
	dibujarGrilla();
	if (cursorPixel) {
		mostrarCursor(cursorPixel.x, cursorPixel.y);
	}
}

function dibujarGrilla() {
	if (!vista) {
		return;
	}
	const spec = GFX[vista.gfx];
	const grid = $("gfx-grid");
	const ctx = grid.getContext("2d");
	ctx.clearRect(0, 0, grid.width, grid.height);
	ctx.strokeStyle = "rgba(255,255,255,0.35)";
	ctx.beginPath();
	for (let x = 0; x <= spec.ancho; x += grilla) {
		const px = Math.min(x * zoom, grid.width - 1) + 0.5;
		ctx.moveTo(px, 0);
		ctx.lineTo(px, spec.alto * zoom);
	}
	for (let y = 0; y <= spec.alto; y += grilla) {
		const py = Math.min(y * zoom, grid.height - 1) + 0.5;
		ctx.moveTo(0, py);
		ctx.lineTo(spec.ancho * zoom, py);
	}
	ctx.stroke();
}

function eventoAPixel(e) {
	const rect = $("gfx-canvas").getBoundingClientRect();
	return {
		x: Math.floor((e.clientX - rect.left) / zoom),
		y: Math.floor((e.clientY - rect.top) / zoom),
	};
}

function acotar(x, y) {
	const spec = GFX[vista.gfx];
	return {
		x: Math.max(0, Math.min(spec.ancho - 1, x)),
		y: Math.max(0, Math.min(spec.alto - 1, y)),
	};
}

function pixelDentro(x, y) {
	const spec = GFX[vista.gfx];
	return x >= 0 && y >= 0 && x < spec.ancho && y < spec.alto;
}

function pintarEn(x, y) {
	if (!vista) {
		return;
	}
	if (leerIndice(vista.gfxBytes, vista.gfx, x, y) === colorIndex) {
		return;
	}
	if (!escribirIndice(vista.gfxBytes, vista.gfx, x, y, colorIndex)) {
		return;
	}
	pintarPixel(x, y);
}

function radioDe(x0, y0, x1, y1) {
	return Math.round(Math.hypot(x1 - x0, y1 - y0));
}

function trazar(tipo, x0, y0, x1, y1, plot) {
	switch (tipo) {
		case "linea":
			linea(x0, y0, x1, y1, plot);
			return;
		case "rect":
			rectangulo(x0, y0, x1, y1, plot);
			return;
		case "rect-lleno":
			rectanguloRelleno(x0, y0, x1, y1, plot);
			return;
		case "circulo":
			circulo(x0, y0, radioDe(x0, y0, x1, y1), plot);
			return;
		case "circulo-lleno":
			circuloRelleno(x0, y0, radioDe(x0, y0, x1, y1), plot);
			return;
		default:
			return;
	}
}

function limpiarPreview() {
	const preview = $("gfx-preview");
	preview.getContext("2d").clearRect(0, 0, preview.width, preview.height);
}

function previsualizar() {
	const preview = $("gfx-preview");
	const ctx = preview.getContext("2d");
	const spec = GFX[vista.gfx];
	const img = ctx.createImageData(spec.ancho, spec.alto);
	const data = img.data;
	let r = 255;
	let g = 255;
	let b = 255;
	let a = 115;
	if (colorIndex !== 0) {
		const color = colorActual();
		r = canal8(color.r);
		g = canal8(color.g);
		b = canal8(color.b);
		a = 255;
	}
	trazar(gesto.herramienta, gesto.x0, gesto.y0, gesto.x1, gesto.y1, (x, y) => {
		if (x < 0 || y < 0 || x >= spec.ancho || y >= spec.alto) {
			return;
		}
		const i = (y * spec.ancho + x) * 4;
		data[i] = r;
		data[i + 1] = g;
		data[i + 2] = b;
		data[i + 3] = a;
	});
	ctx.putImageData(img, 0, 0);
}

function aplicarForma() {
	trazar(gesto.herramienta, gesto.x0, gesto.y0, gesto.x1, gesto.y1, (x, y) => {
		escribirIndice(vista.gfxBytes, vista.gfx, x, y, colorIndex);
	});
	redibujarHoja();
}

function finalizarGesto(escribir) {
	if (!gesto) {
		return;
	}
	const aplicar = escribir && vista;
	if (aplicar && gesto.herramienta !== "lapiz") {
		aplicarForma();
	}
	if (aplicar && !bytesIguales(gesto.antes, vista.gfxBytes)) {
		comprometer();
		deshacerPila.push(gesto.antes);
		if (deshacerPila.length > HISTORIA_MAX) {
			deshacerPila.shift();
		}
		rehacerPila.length = 0;
	}
	gesto = null;
	limpiarPreview();
	actualizarHistoria();
}

function tomarColor(x, y) {
	if (!pixelDentro(x, y)) {
		return;
	}
	const idx = leerIndice(vista.gfxBytes, vista.gfx, x, y);
	const spec = GFX[vista.gfx];
	if (idx >= spec.colores) {
		return;
	}
	colorIndex = idx;
	pintarColores();
}

function seleccionarHerramienta(nombre) {
	herramienta = nombre;
	for (const btn of $("gfx-tools").querySelectorAll("[data-herramienta]")) {
		btn.classList.toggle("active", btn.dataset.herramienta === nombre);
	}
}

function ocultarCursor() {
	$("gfx-cursor").hidden = true;
}

function olvidarCursor() {
	cursorPixel = null;
	ocultarCursor();
}

function mostrarCursor(x, y) {
	cursorPixel = {x, y};
	if (!vista || zoom <= 4 || !pixelDentro(x, y)) {
		ocultarCursor();
		return;
	}
	const el = $("gfx-cursor");
	el.hidden = false;
	el.style.width = `${zoom}px`;
	el.style.height = `${zoom}px`;
	el.style.transform = `translate(${x * zoom}px, ${y * zoom}px)`;
}

function focoEnCampo() {
	const tag = document.activeElement?.tagName;
	return tag === "INPUT" || tag === "SELECT" || tag === "TEXTAREA";
}

export function initGraficos() {
	const paletas = $("gfx-paleta");
	for (let i = 0; i < 8; i++) {
		const op = document.createElement("option");
		op.value = String(i);
		op.textContent = `Paleta ${i}`;
		paletas.append(op);
	}

	paletas.addEventListener("change", () => {
		if (!vista) {
			return;
		}
		paletaIndex = Number(paletas.value);
		const spec = GFX[vista.gfx];
		if (colorIndex >= spec.colores) {
			colorIndex = 0;
		}
		pintarColores();
		redibujarHoja();
	});

	for (const id of ["gfx-r", "gfx-g", "gfx-b"]) {
		$(id).addEventListener("input", aplicarRgb);
	}

	$("gfx-zoom").addEventListener("change", () => {
		zoom = Number($("gfx-zoom").value);
		if (vista) {
			aplicarZoom();
		}
	});

	$("gfx-grilla").addEventListener("change", () => {
		grilla = Number($("gfx-grilla").value);
		dibujarGrilla();
	});

	$("gfx-tools").addEventListener("click", (e) => {
		const btn = e.target.closest("[data-herramienta]");
		if (!btn) {
			return;
		}
		seleccionarHerramienta(btn.dataset.herramienta);
	});
	$("gfx-undo").addEventListener("click", deshacer);
	$("gfx-redo").addEventListener("click", rehacer);

	const canvas = $("gfx-canvas");
	canvas.addEventListener("pointerdown", (e) => {
		if (e.button !== 0 || !vista) {
			return;
		}
		const p = eventoAPixel(e);
		mostrarCursor(p.x, p.y);
		if (herramienta === "tomar") {
			tomarColor(p.x, p.y);
			return;
		}
		const origen = acotar(p.x, p.y);
		gesto = {
			herramienta,
			x0: origen.x,
			y0: origen.y,
			x1: origen.x,
			y1: origen.y,
			antes: copiarGfx(),
		};
		canvas.setPointerCapture(e.pointerId);
		if (herramienta === "lapiz") {
			pintarEn(origen.x, origen.y);
		} else {
			previsualizar();
		}
		actualizarHistoria();
	});
	canvas.addEventListener("pointermove", (e) => {
		const p = eventoAPixel(e);
		mostrarCursor(p.x, p.y);
		if (!gesto) {
			return;
		}
		if (gesto.herramienta === "lapiz") {
			pintarEn(p.x, p.y);
			return;
		}
		const fin = acotar(p.x, p.y);
		gesto.x1 = fin.x;
		gesto.y1 = fin.y;
		previsualizar();
	});
	const soltar = () => finalizarGesto(true);
	canvas.addEventListener("pointerup", soltar);
	canvas.addEventListener("pointercancel", soltar);
	canvas.addEventListener("pointerleave", () => {
		if (!gesto) {
			olvidarCursor();
		}
	});

	$("gfx-scroll").addEventListener(
		"wheel",
		(e) => {
			if (!vista) {
				return;
			}
			e.preventDefault();
			let i = ZOOMS.indexOf(zoom);
			if (i < 0) {
				i = 0;
			}
			const siguiente = e.deltaY < 0 ? Math.min(ZOOMS.length - 1, i + 1) : Math.max(0, i - 1);
			if (ZOOMS[siguiente] === zoom) {
				return;
			}
			zoom = ZOOMS[siguiente];
			$("gfx-zoom").value = String(zoom);
			aplicarZoom();
		},
		{passive: false},
	);

	window.addEventListener(
		"keydown",
		(e) => {
			if ($("gfx-editor").hidden || focoEnCampo()) {
				return;
			}
			const mod = e.ctrlKey || e.metaKey;
			if (!mod || e.altKey) {
				return;
			}
			const key = e.key.toLowerCase();
			if (key === "z" && !e.shiftKey) {
				e.preventDefault();
				e.stopPropagation();
				deshacer();
			} else if (key === "y" || (key === "z" && e.shiftKey)) {
				e.preventDefault();
				e.stopPropagation();
				rehacer();
			}
		},
		true,
	);

	subscribe(sincronizar);
	sincronizar();
}
