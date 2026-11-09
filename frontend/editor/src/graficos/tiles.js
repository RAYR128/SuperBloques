export const TILES_POR_FILA = 16;

export const GFX = {
	principal: {bpp: 4, ancho: 128, alto: 512, bytes: 32768, colores: 16, bytesPaleta: 32},
	hud: {bpp: 2, ancho: 128, alto: 128, bytes: 4096, colores: 4, bytesPaleta: 8},
	objetos: {bpp: 4, ancho: 128, alto: 256, bytes: 16384, colores: 16, bytesPaleta: 32},
};

export function bytesDeBase64(b64) {
	const bin = atob(b64);
	const out = new Uint8Array(bin.length);
	for (let i = 0; i < bin.length; i++) {
		out[i] = bin.charCodeAt(i);
	}
	return out;
}

export function bytesABase64(bytes) {
	const chunk = 0x8000;
	let binary = "";
	for (let i = 0; i < bytes.length; i += chunk) {
		binary += String.fromCharCode.apply(null, bytes.subarray(i, i + chunk));
	}
	return btoa(binary);
}

export function localizar(gfx, px, py) {
	const spec = GFX[gfx];
	if (px < 0 || py < 0 || px >= spec.ancho || py >= spec.alto) {
		return null;
	}
	const tileIndex = (py >> 3) * TILES_POR_FILA + (px >> 3);
	const bytesPorTile = spec.bpp === 4 ? 32 : 16;
	return {
		offset: tileIndex * bytesPorTile,
		yLocal: py & 7,
		bit: 7 - (px & 7),
		bpp: spec.bpp,
	};
}

export function leerIndice(bytes, gfx, px, py) {
	const loc = localizar(gfx, px, py);
	if (!loc) {
		return 0;
	}
	const {offset, yLocal, bit, bpp} = loc;
	const b0 = bytes[offset + yLocal * 2];
	const b1 = bytes[offset + yLocal * 2 + 1];
	let idx = ((b0 >> bit) & 1) | (((b1 >> bit) & 1) << 1);
	if (bpp === 4) {
		const b2 = bytes[offset + 16 + yLocal * 2];
		const b3 = bytes[offset + 16 + yLocal * 2 + 1];
		idx |= ((b2 >> bit) & 1) << 2;
		idx |= ((b3 >> bit) & 1) << 3;
	}
	return idx;
}

function escribirBit(bytes, addr, bit, activo) {
	const mask = 1 << bit;
	if (activo) {
		bytes[addr] |= mask;
	} else {
		bytes[addr] &= ~mask;
	}
}

export function escribirIndice(bytes, gfx, px, py, indice) {
	const loc = localizar(gfx, px, py);
	if (!loc) {
		return false;
	}
	const {offset, yLocal, bit, bpp} = loc;
	const planos = bpp === 4 ? 4 : 2;
	for (let p = 0; p < planos; p++) {
		const base = p < 2 ? offset + yLocal * 2 : offset + 16 + yLocal * 2;
		escribirBit(bytes, base + (p & 1), bit, (indice >> p) & 1);
	}
	return true;
}

export function offsetColor(gfx, paleta, indice) {
	const spec = GFX[gfx];
	return paleta * spec.bytesPaleta + indice * 2;
}

export function color15(bytes, offset) {
	const c = bytes[offset] | (bytes[offset + 1] << 8);
	return {r: c & 31, g: (c >> 5) & 31, b: (c >> 10) & 31};
}

export function escribirColor15(bytes, offset, r, g, b) {
	const c = (r & 31) | ((g & 31) << 5) | ((b & 31) << 10);
	bytes[offset] = c & 255;
	bytes[offset + 1] = (c >> 8) & 255;
}

export function canal8(v) {
	return (v << 3) | (v >> 2);
}

export function rgbCss(r, g, b) {
	return `rgb(${canal8(r)}, ${canal8(g)}, ${canal8(b)})`;
}

export function renderHoja(gfxBytes, paletaBytes, gfx, paleta, imageData) {
	const spec = GFX[gfx];
	const data = imageData.data;
	const base = paleta * spec.bytesPaleta;
	for (let py = 0; py < spec.alto; py++) {
		for (let px = 0; px < spec.ancho; px++) {
			const di = (py * spec.ancho + px) * 4;
			const idx = leerIndice(gfxBytes, gfx, px, py);
			if (idx === 0) {
				data[di] = 0;
				data[di + 1] = 0;
				data[di + 2] = 0;
				data[di + 3] = 0;
				continue;
			}
			const color = color15(paletaBytes, base + idx * 2);
			data[di] = canal8(color.r);
			data[di + 1] = canal8(color.g);
			data[di + 2] = canal8(color.b);
			data[di + 3] = 255;
		}
	}
}
