import bateriaUrl from "../defaults/snds/Bateria.brr?inline";
import cuerdaUrl from "../defaults/snds/Cuerda.brr?inline";
import synthUrl from "../defaults/snds/Synth.brr?inline";
import trompetaUrl from "../defaults/snds/Trompeta.brr?inline";
import {validarBrr} from "./brr.js";

function bytesDeDataUrl(dataUrl) {
	const i = dataUrl.indexOf(",");
	const b64 = i >= 0 ? dataUrl.slice(i + 1) : dataUrl;
	const bin = atob(b64);
	const out = new Uint8Array(bin.length);
	for (let n = 0; n < bin.length; n++) {
		out[n] = bin.charCodeAt(n);
	}
	return out;
}

export const LIBRERIA = [
	{nombre: "Bateria", bytes: bytesDeDataUrl(bateriaUrl)},
	{nombre: "Cuerda", bytes: bytesDeDataUrl(cuerdaUrl)},
	{nombre: "Synth", bytes: bytesDeDataUrl(synthUrl)},
	{nombre: "Trompeta", bytes: bytesDeDataUrl(trompetaUrl)},
];

for (const item of LIBRERIA) {
	validarBrr(item.bytes);
}
