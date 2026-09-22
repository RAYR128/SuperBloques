import {compilarRom} from "./rom.js";

const SAMPLE_RATE = 36000;
const TARGET_FRAME_MS = 1000 / 60;
const AUDIO_BUFFER_SAMPLES = 2048;
const SNES_W = 256;
const SNES_H = 224;
const SRC_W = 512;
const FB_W = 512;
const FB_H = 448;

const KEY_BITS = {
	ArrowRight: 8,
	ArrowLeft: 9,
	ArrowDown: 10,
	ArrowUp: 11,
	a: 7,
	z: 15,
	x: 6,
	s: 14,
	d: 5,
	c: 4,
	Enter: 12,
	Shift: 13,
};

let moduloPromise = null;
let emu = null;
let corriendo = false;
let compilando = false;
let keyInput = 0;
let raf = 0;
let lastFrameTime = 0;
let frameAccumulator = 0;
let visible = true;
let ac = null;
let sinAudio = false;
let ctx = null;
let lowCanvas = null;
let lowCtx = null;
let lowData = null;
let stage = null;

function informar(html) {
	const el = document.querySelector(".debug-body");
	if (el) {
		el.innerHTML = html;
	}
}

function escapar(texto) {
	return String(texto)
		.replace(/&/g, "&amp;")
		.replace(/</g, "&lt;")
		.replace(/>/g, "&gt;");
}

function cargarModulo() {
	if (!moduloPromise) {
		const glue = new URL(`${import.meta.env.BASE_URL}emu/snes9x.js`, location.origin).href;
		moduloPromise = import(/* @vite-ignore */ glue)
			.then((mod) => {
				const factory = mod.default ?? mod;
				return factory({
					locateFile: (path) => new URL(path, glue).href,
				});
			})
			.catch((e) => {
				moduloPromise = null;
				throw e;
			});
	}
	return moduloPromise;
}

function bitDeTecla(key) {
	if (Object.prototype.hasOwnProperty.call(KEY_BITS, key)) {
		return KEY_BITS[key];
	}
	if (key.length === 1) {
		const baja = key.toLowerCase();
		if (Object.prototype.hasOwnProperty.call(KEY_BITS, baja)) {
			return KEY_BITS[baja];
		}
	}
	return null;
}

function teclaIgnorada(e) {
	const t = e.target;
	return t instanceof Element && Boolean(t.closest("input, textarea, select"));
}

function alBajarTecla(e) {
	if (!corriendo || teclaIgnorada(e)) {
		return;
	}
	const bit = bitDeTecla(e.key);
	if (bit === null) {
		return;
	}
	e.preventDefault();
	e.stopPropagation();
	keyInput |= 1 << bit;
}

function alSubirTecla(e) {
	if (!corriendo || teclaIgnorada(e)) {
		return;
	}
	const bit = bitDeTecla(e.key);
	if (bit === null) {
		return;
	}
	e.preventDefault();
	e.stopPropagation();
	keyInput &= ~(1 << bit);
}

function pintarBoton(modo) {
	const btn = document.getElementById("stage-play");
	if (!btn) {
		return;
	}
	const detenido = modo !== "stop";
	btn.classList.toggle("is-stop", !detenido);
	btn.disabled = modo === "busy";
	const etiqueta = detenido ? "Reproducir" : "Detener";
	btn.setAttribute("aria-label", etiqueta);
	btn.title = etiqueta;
}

function asegurarAudio() {
	if (ac || sinAudio) {
		return;
	}
	const AudioContext = window.AudioContext || window.webkitAudioContext;
	if (!AudioContext) {
		sinAudio = true;
		return;
	}
	try {
		ac = new AudioContext({sampleRate: SAMPLE_RATE});
	} catch {
		sinAudio = true;
		return;
	}
	const nodo = ac.createScriptProcessor(AUDIO_BUFFER_SAMPLES, 0, 2);
	nodo.onaudioprocess = (e) => {
		const izq = e.outputBuffer.getChannelData(0);
		const der = e.outputBuffer.getChannelData(1);
		if (!corriendo || !visible || !emu) {
			izq.fill(0);
			der.fill(0);
			return;
		}
		const ptr = emu._getSoundBuffer();
		if (!ptr) {
			izq.fill(0);
			der.fill(0);
			return;
		}
		const muestras = new Float32Array(emu.HEAPF32.buffer, ptr, AUDIO_BUFFER_SAMPLES * 2);
		izq.set(muestras.subarray(0, AUDIO_BUFFER_SAMPLES));
		der.set(muestras.subarray(AUDIO_BUFFER_SAMPLES));
	};
	nodo.connect(ac.destination);
}

function cuadro() {
	if (!corriendo || !visible || !emu || !ctx || !lowCtx || !lowData) {
		return;
	}
	emu._setJoypadInput(keyInput);
	emu._mainLoop();
	const ptr = emu._getScreenBuffer();
	if (!ptr) {
		return;
	}
	const src = new Uint8Array(emu.HEAPU8.buffer, ptr, SRC_W * FB_H * 4);
	const dst = lowData.data;
	for (let y = 0; y < SNES_H; y++) {
		const desde = y * SRC_W * 4;
		dst.set(src.subarray(desde, desde + SNES_W * 4), y * SNES_W * 4);
	}
	lowCtx.putImageData(lowData, 0, 0);
	ctx.imageSmoothingEnabled = false;
	ctx.drawImage(lowCanvas, 0, 0, FB_W, FB_H);
}

function pedirCuadro(timestamp) {
	if (!lastFrameTime) {
		lastFrameTime = timestamp;
	}
	if (visible && corriendo) {
		frameAccumulator += timestamp - lastFrameTime;
		if (frameAccumulator > TARGET_FRAME_MS * 5) {
			frameAccumulator = TARGET_FRAME_MS * 5;
		}
		while (frameAccumulator >= TARGET_FRAME_MS) {
			cuadro();
			frameAccumulator -= TARGET_FRAME_MS;
		}
	} else {
		frameAccumulator = 0;
	}
	lastFrameTime = timestamp;
	raf = requestAnimationFrame(pedirCuadro);
}

function detener() {
	corriendo = false;
	keyInput = 0;
	if (emu) {
		emu._stopEmulator();
	}
	if (raf) {
		cancelAnimationFrame(raf);
		raf = 0;
	}
	frameAccumulator = 0;
	lastFrameTime = 0;
	pintarBoton("play");
}

async function reproducir() {
	if (compilando || corriendo) {
		return;
	}
	compilando = true;
	pintarBoton("busy");
	try {
		asegurarAudio();
		if (ac && ac.state === "suspended") {
			await ac.resume();
		}
		const rom = await compilarRom();
		if (!rom) {
			pintarBoton("play");
			return;
		}
		let modulo;
		try {
			modulo = await cargarModulo();
		} catch {
			informar('<div class="warn">[error] no esta el emulador WASM</div>');
			pintarBoton("play");
			return;
		}
		emu = modulo;
		const ptr = emu._malloc(rom.length);
		if (!ptr) {
			informar('<div class="warn">[error] sin memoria para la ROM</div>');
			pintarBoton("play");
			return;
		}
		try {
			emu.HEAPU8.set(rom, ptr);
			const ok = emu._startWithRom(ptr, rom.length, SAMPLE_RATE);
			if (!ok) {
				informar('<div class="warn">[error] el emulador no pudo cargar la ROM</div>');
				pintarBoton("play");
				return;
			}
		} finally {
			emu._free(ptr);
		}
		corriendo = true;
		keyInput = 0;
		frameAccumulator = 0;
		lastFrameTime = 0;
		pintarBoton("stop");
		stage?.focus({preventScroll: true});
		raf = requestAnimationFrame(pedirCuadro);
		informar(`<div class="ok">[ok] emulador en marcha</div><div class="dim">${rom.length} bytes</div>`);
	} catch (e) {
		const msg = e instanceof Error ? e.message : String(e);
		informar(`<div class="warn">[error] ${escapar(msg)}</div>`);
		detener();
	} finally {
		compilando = false;
		if (!corriendo) {
			pintarBoton("play");
		}
	}
}

export function initEmu() {
	stage = document.getElementById("stage");
	const btn = document.getElementById("stage-play");
	if (!stage || !btn) {
		return;
	}
	ctx = stage.getContext("2d", {alpha: false});
	ctx.imageSmoothingEnabled = false;
	lowCanvas = document.createElement("canvas");
	lowCanvas.width = SNES_W;
	lowCanvas.height = SNES_H;
	lowCtx = lowCanvas.getContext("2d", {alpha: false});
	lowData = lowCtx.createImageData(SNES_W, SNES_H);
	btn.addEventListener("click", () => {
		if (corriendo) {
			detener();
		} else {
			void reproducir();
		}
	});
	window.addEventListener("keydown", alBajarTecla, true);
	window.addEventListener("keyup", alSubirTecla, true);
	document.addEventListener("visibilitychange", () => {
		visible = !document.hidden;
		frameAccumulator = 0;
		lastFrameTime = 0;
	});
}
