export const BRR_ARCHIVO_MAX = 32768;
export const BRR_DATOS_MAX = 32760;
export const SONIDOS_MAX = 32;

export function validarBrr(bytes) {
	const datos = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes);
	if (datos.length > BRR_ARCHIVO_MAX) {
		throw new Error("El BRR supera 32KB");
	}
	if (datos.length < 11 || (datos.length - 2) % 9 !== 0) {
		throw new Error("El BRR debe medir 2 + (bloques * 9) bytes");
	}
	const puntoLoop = datos[0] | (datos[1] << 8);
	const sample = datos.subarray(2);
	if (puntoLoop % 9 !== 0 || puntoLoop >= sample.length) {
		throw new Error("El punto de loop no cae en un bloque");
	}
	return {datos: sample, puntoLoop};
}

export function bytesABase64(bytes) {
	const chunk = 0x8000;
	let binary = "";
	for (let i = 0; i < bytes.length; i += chunk) {
		binary += String.fromCharCode.apply(null, bytes.subarray(i, i + chunk));
	}
	return btoa(binary);
}

export function bytesDeBase64(entrada) {
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
	let pad = 0;
	if (limpia.endsWith("==")) {
		pad = 2;
	} else if (limpia.endsWith("=")) {
		pad = 1;
	}
	return (limpia.length / 4) * 3 - pad;
}
