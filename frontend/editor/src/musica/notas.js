export const FRECUENCIA_C4 = 768;
export const NOTA_MAX = 111;
export const CANALES = ["Canal1", "Canal2", "Canal3", "Canal4", "Canal5", "Canal6", "Canal7", "Canal8"];
export const SNAP_TICKS = 48;
export const DURACION_INICIAL = 192;
export const DURACION_MIN = 48;

const NOMBRES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];

export function nombreNota(nota) {
	const n = ((nota % 12) + 12) % 12;
	const oct = Math.floor(nota / 12);
	return `${NOMBRES[n]}-${oct}`;
}

export function frecuenciaDeNota(semitonosDesdeC0) {
	return Math.round(FRECUENCIA_C4 * 2 ** ((semitonosDesdeC0 - 48) / 12));
}

export const NOTAS_FRECUENCIA = (() => {
	const salida = [];
	for (let oct = 0; oct <= 8; oct++) {
		for (let n = 0; n < 12; n++) {
			const semi = oct * 12 + n;
			const frecuencia = frecuenciaDeNota(semi);
			if (frecuencia < 0 || frecuencia > 65535) {
				continue;
			}
			salida.push({nombre: `${NOMBRES[n]}-${oct}`, frecuencia});
		}
	}
	return salida;
})();

export function notaDeFrecuencia(frecuencia) {
	return NOTAS_FRECUENCIA.find((n) => n.frecuencia === frecuencia) ?? null;
}

export const ATAQUE = [
	"4.096 segundos",
	"2.560 segundos",
	"1.536 segundos",
	"1.024 segundos",
	"0.640 segundos",
	"0.384 segundos",
	"0.256 segundos",
	"0.160 segundos",
	"0.096 segundos",
	"0.064 segundos",
	"0.040 segundos",
	"0.024 segundos",
	"0.016 segundos",
	"0.010 segundos",
	"0.006 segundos",
	"0.002 segundos",
];

export const DECAY = [
	"0.880 segundos",
	"0.550 segundos",
	"0.330 segundos",
	"0.220 segundos",
	"0.138 segundos",
	"0.083 segundos",
	"0.055 segundos",
	"0.028 segundos",
];

export const SUSTAIN = ["12.5%", "25%", "37.5%", "50%", "62.5%", "75%", "87.5%", "100%"];

export const RELEASE = [
	"No",
	"44.480 segundos",
	"33.360 segundos",
	"27.800 segundos",
	"22.240 segundos",
	"16.680 segundos",
	"13.900 segundos",
	"11.120 segundos",
	"8.340 segundos",
	"6.950 segundos",
	"5.560 segundos",
	"4.170 segundos",
	"3.475 segundos",
	"2.780 segundos",
	"2.085 segundos",
	"1.738 segundos",
	"1.390 segundos",
	"1.042 segundos",
	"0.869 segundos",
	"0.695 segundos",
	"0.521 segundos",
	"0.434 segundos",
	"0.347 segundos",
	"0.261 segundos",
	"0.217 segundos",
	"0.174 segundos",
	"0.130 segundos",
	"0.109 segundos",
	"0.087 segundos",
	"0.065 segundos",
	"0.043 segundos",
	"0.022 segundos",
];

export function etiquetaAdsR(valor, texto) {
	return `${valor.toString(16).toUpperCase().padStart(2, "0")}  ${texto}`;
}

export function colorDeInstrumento(nombre) {
	let h = 0;
	for (let i = 0; i < nombre.length; i++) {
		h = (h * 33 + nombre.charCodeAt(i)) >>> 0;
	}
	return `hsl(${h % 360} 70% 52%)`;
}
