import {state, subscribe} from "../project.js";
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

const ZOOMS = [1, 2, 4, 8];

let vista = null;
let clave = "";
let paletaIndex = 0;
let colorIndex = 1;
let zoom = 4;
let grilla = 8;
let pintando = false;

function $(id) {
	return document.getElementById(id);
}

function claveDe(sel) {
	if (!sel || sel.tipo !== "gfx") {
		return "";
	}
	return `${state.generation}:${sel.gfx}:${sel.nombre}`;
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
	aplicarZoom();
	pintarColores();
	redibujarHoja();
}

function sincronizar() {
	const siguiente = claveDe(state.seleccion);
	if (siguiente === clave) {
		return;
	}
	clave = siguiente;
	if (!siguiente) {
		vista = null;
		pintando = false;
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
	const grid = $("gfx-grid");
	const stage = $("gfx-stage");
	canvas.style.width = `${w}px`;
	canvas.style.height = `${h}px`;
	stage.style.width = `${w}px`;
	stage.style.height = `${h}px`;
	grid.width = w;
	grid.height = h;
	dibujarGrilla();
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
	vista.escribirGfx(bytesABase64(vista.gfxBytes));
	pintarPixel(x, y);
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

	const canvas = $("gfx-canvas");
	canvas.addEventListener("pointerdown", (e) => {
		if (e.button !== 0 || !vista) {
			return;
		}
		pintando = true;
		canvas.setPointerCapture(e.pointerId);
		const p = eventoAPixel(e);
		pintarEn(p.x, p.y);
	});
	canvas.addEventListener("pointermove", (e) => {
		if (!pintando) {
			return;
		}
		const p = eventoAPixel(e);
		pintarEn(p.x, p.y);
	});
	const soltar = () => {
		pintando = false;
	};
	canvas.addEventListener("pointerup", soltar);
	canvas.addEventListener("pointercancel", soltar);

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

	subscribe(sincronizar);
	sincronizar();
}
