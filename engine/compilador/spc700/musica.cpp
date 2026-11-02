// Generacion de datos de musica para el SPC700
#include "asm/op.h"
#include "spc700.h"
#include <algorithm>
#include <array>
#include <climits>
#include <stdexcept>
#include <iostream>

std::vector<InstrumentoMusica> InstrumentosProyecto;
std::vector<SonidoProyecto> SonidosProyecto;
std::vector<MusicaProyecto> MusicasProyecto;

void InstrumentoMusica::EmpaquetarADSR(uint8_t Salida[3]) const {
	Salida[0] = (uint8_t)((EsGain ? 0x80 : 0x00) + (D * 0x10) + A);
	Salida[1] = (uint8_t)((S * 0x20) + R);
	Salida[2] = EsGain ? Gain : (uint8_t)0x7F;
}

// el driver usa 0x01-0x7F como duracion, 0x80+ son comandos
#define DURACION_MAXIMA_TICK 0x7F

// tramo continuo de tiempo dentro de un canal: una nota o un silencio
struct TramoCanal {
	bool EsNota;
	uint8_t Nota;
	int Instrumento;
	int Duracion;
};

// forma de dividir un tramo en trozos: (Duracion / C) trozos de C ticks mas un resto R (0 <= R < C)
// RestoPrimero indica si el resto va antes o despues de los trozos de C
// DuracionAnterior es el registro de duracion antes del tramo (para reconstruir el camino optimo)
struct DivisionTramo {
	uint8_t C = 0;
	uint8_t R = 0;
	bool RestoPrimero = false;
	uint8_t DuracionAnterior = 0;
};

std::vector<int> TrozosDivision(int Duracion, const DivisionTramo &Division) {
	std::vector<int> trozos;
	if(Division.R && Division.RestoPrimero) {
		trozos.push_back(Division.R);
	}
	trozos.insert(trozos.end(), Duracion / Division.C, Division.C);
	if(Division.R && !Division.RestoPrimero) {
		trozos.push_back(Division.R);
	}
	return trozos;
}

// el registro de duracion del driver es persistente, asi que solo cuesta un byte cuando cambia.
// programacion dinamica sobre el valor del registro despues de cada tramo para minimizar los bytes del canal.
// el estado 0 es "duracion desconocida" (inicio del canal), nunca coincide con un trozo.
std::vector<DivisionTramo> OptimizarDuraciones(const std::vector<TramoCanal> &Tramos) {
	const int Infinito = INT_MAX / 2;
	std::array<int, DURACION_MAXIMA_TICK + 1> costo;
	costo.fill(Infinito);
	costo[0] = 0;
	std::vector<std::array<DivisionTramo, DURACION_MAXIMA_TICK + 1>> pasos(Tramos.size());

	for(size_t t = 0; t < Tramos.size(); t++) {
		int duracion = Tramos[t].Duracion;
		int estadoMinimo = (int)(std::min_element(costo.begin(), costo.end()) - costo.begin());
		std::array<int, DURACION_MAXIMA_TICK + 1> nuevo;
		nuevo.fill(Infinito);
		for(int c = 1; c <= std::min(duracion, DURACION_MAXIMA_TICK); c++) {
			int q = duracion / c;
			int r = duracion % c;
			// 1 byte de comando por trozo + bytes de duracion asumiendo que el primer trozo cambia el registro
			int base = q + 1 + (r ? 2 : 0);
			for(int restoPrimero = 0; restoPrimero <= (r ? 1 : 0); restoPrimero++) {
				int primero = restoPrimero ? r : c;
				int ultimo = (r && !restoPrimero) ? r : c;
				int anterior = estadoMinimo;
				int total = costo[estadoMinimo] + base;
				// si el registro ya tiene la duracion del primer trozo nos ahorramos un byte
				if(costo[primero] + base - 1 < total) {
					anterior = primero;
					total = costo[primero] + base - 1;
				}
				if(total < nuevo[ultimo]) {
					nuevo[ultimo] = total;
					pasos[t][ultimo] = {(uint8_t)c, (uint8_t)r, restoPrimero != 0, (uint8_t)anterior};
				}
			}
		}
		costo = nuevo;
	}

	// reconstruir el camino optimo desde el final
	std::vector<DivisionTramo> divisiones(Tramos.size());
	int estado = (int)(std::min_element(costo.begin(), costo.end()) - costo.begin());
	for(size_t t = Tramos.size(); t-- > 0;) {
		divisiones[t] = pasos[t][estado];
		estado = divisiones[t].DuracionAnterior;
	}
	return divisiones;
}

void MusicaProyecto::Ensamblar() {
	if(Instrumentos.size() > 256) {
		throw std::runtime_error("Musica " + Nombre + " tiene mas de 256 instrumentos");
	}

	// calcular la duracion total de la cancion antes de ensamblar
	int duracionMusicaTicks = 0;
	for(size_t canal = 0; canal < 8; canal++) {
		for(NotaMusica &nota : NotasCanal[canal]) {
			if(nota.Posicion < 0) {
				throw std::runtime_error("Nota en canal " + std::to_string(canal) + " tiene posicion negativa");
			}
			if(nota.Duracion <= 0) {
				throw std::runtime_error("Nota en canal " + std::to_string(canal) + " tiene duracion no positiva");
			}
			if(nota.Instrumento < 0 || nota.Instrumento >= (int)Instrumentos.size()) {
				throw std::runtime_error("Nota en canal " + std::to_string(canal) + " tiene instrumento invalido (" + std::to_string(nota.Instrumento) + ")");
			}
			if(nota.Posicion + nota.Duracion > duracionMusicaTicks) {
				duracionMusicaTicks = nota.Posicion + nota.Duracion;
			}
		}
	}

	// ejemplo si duracionMusicaTicks = 128 y canal 0 tiene una nota 0x30 de 32 ticks, generariamos algo como
	// [0xFC (SB_ACMD_INSTRUMENTO)] [0x00 (NotaMusica.Instrumento)] (solo cuando el instrumento es diferente para el canal actual)
	// [0x20 (SB_ACMD_DURACION)] [0x30 (NotaMusica.Nota) + SB_ACMD_NOTA] [0x60 (SB_ACMD_DURACION)] [0xFE (SB_ACMD_SILENCIO)] [0x00 (SB_ACMD_FINAL_DE_CANAL)]
	for(size_t canal = 0; canal < 8; canal++) {
		std::stable_sort(NotasCanal[canal].begin(), NotasCanal[canal].end(), [](const NotaMusica &a, const NotaMusica &b) {
			return a.Posicion < b.Posicion;
		});

		// cada canal es monofonico: si una nota se solapa con la siguiente se recorta (o se descarta si empiezan juntas)
		std::vector<NotaMusica> notas;
		for(const NotaMusica &nota : NotasCanal[canal]) {
			if(!notas.empty()) {
				NotaMusica &previa = notas.back();
				if(previa.Posicion + previa.Duracion > nota.Posicion) {
					std::string aviso = "Advertencia: musica " + Nombre + ", canal " + std::to_string(canal) + ": nota " + std::to_string(previa.Nota) + " (tick " + std::to_string(previa.Posicion) + ", duracion " + std::to_string(previa.Duracion) + ") se solapa con nota " + std::to_string(nota.Nota) + " (tick " + std::to_string(nota.Posicion) + ")";
					if(previa.Posicion == nota.Posicion) {
						std::cerr << aviso << ", se descarta la primera" << std::endl;
						notas.pop_back();
					} else {
						std::cerr << aviso << ", se recorta a " << (nota.Posicion - previa.Posicion) << " ticks" << std::endl;
						previa.Duracion = nota.Posicion - previa.Posicion;
					}
				}
			}
			notas.push_back(nota);
		}

		// convertir a tramos de nota/silencio continuos
		std::vector<TramoCanal> tramos;
		int cursor = 0;
		for(const NotaMusica &nota : notas) {
			if(nota.Posicion > cursor) {
				tramos.push_back({false, 0, -1, nota.Posicion - cursor});
			}
			tramos.push_back({true, nota.Nota, nota.Instrumento, nota.Duracion});
			cursor = nota.Posicion + nota.Duracion;
		}
		// rellenar con silencio hasta el final para que todos los canales hagan loop al mismo tiempo
		if(duracionMusicaTicks > cursor) {
			tramos.push_back({false, 0, -1, duracionMusicaTicks - cursor});
		}
		// un canal que solo tiene 0x00 deja al driver en un bucle infinito
		if(tramos.empty()) {
			tramos.push_back({false, 0, -1, DURACION_MAXIMA_TICK});
		}

		std::vector<DivisionTramo> divisiones = OptimizarDuraciones(tramos);

		// el driver arranca con instrumento 0 y al hacer loop queda el de la ultima nota,
		// solo podemos omitir el primer cambio si ambos son 0
		int InstrumentoActual = (notas.empty() || notas.back().Instrumento == 0) ? 0 : -1;

		// la duracion es desconocida al comenzar el canal (0 en la primera pasada, la ultima usada al hacer loop)
		int DuracionActual = 0;
		std::vector<uint8_t> &datos = DatosCanal[canal];
		datos.clear();
		for(size_t t = 0; t < tramos.size(); t++) {
			const TramoCanal &tramo = tramos[t];
			if(tramo.EsNota && tramo.Instrumento != InstrumentoActual) {
				datos.push_back(SB_ACMD_INSTRUMENTO);
				datos.push_back((uint8_t)tramo.Instrumento);
				InstrumentoActual = tramo.Instrumento;
			}
			std::vector<int> trozos = TrozosDivision(tramo.Duracion, divisiones[t]);
			for(size_t i = 0; i < trozos.size(); i++) {
				if(trozos[i] != DuracionActual) {
					datos.push_back((uint8_t)trozos[i]);
					DuracionActual = trozos[i];
				}
				// la nota se extiende con REST (no hace key off), el silencio se puede repetir
				if(!tramo.EsNota) {
					datos.push_back(SB_ACMD_SILENCIO);
				} else if(i == 0) {
					datos.push_back((uint8_t)(SB_ACMD_NOTA + tramo.Nota));
				} else {
					datos.push_back(SB_ACMD_REST);
				}
			}
		}
		datos.push_back(SB_ACMD_FINAL_DE_CANAL);
	}
}

int MusicaProyecto::Generar(int InicioDatos) {
	// generar 8 punteros 16-bit LE hacia cada canal
	int TamanoInstrumentos = Instrumentos.size() * 6;
	GeneracionFinal.clear();
	int PtrCanal = InicioDatos + 8 * 2 + TamanoInstrumentos;
	for(size_t canal = 0; canal < 8; canal++) {
		GeneracionFinal.push_back(PtrCanal & 0xFF);
		GeneracionFinal.push_back((PtrCanal >> 8) & 0xFF);
		PtrCanal += DatosCanal[canal].size();
	}
	// instrumentos
	// sample, adsr 1/2/3, frecuencia (LE)
	for(size_t inst = 0; inst < Instrumentos.size(); inst++) {
		const InstrumentoMusica &i = Instrumentos[inst];
		if(i.Sonido < 0 || i.Sonido >= (int)SonidosProyecto.size()) {
			throw std::runtime_error("Instrumento " + i.Nombre + " tiene referencia a sonido invalido");
		}
		uint8_t adsr[3];
		i.EmpaquetarADSR(adsr);
		GeneracionFinal.push_back((uint8_t)i.Sonido);
		GeneracionFinal.push_back(adsr[0]);
		GeneracionFinal.push_back(adsr[1]);
		GeneracionFinal.push_back(adsr[2]);
		GeneracionFinal.push_back(i.Frecuencia & 0xFF);
		GeneracionFinal.push_back((i.Frecuencia >> 8) & 0xFF);
	}
	// insertar datos
	for(size_t canal = 0; canal < 8; canal++) {
		GeneracionFinal.insert(GeneracionFinal.end(), DatosCanal[canal].begin(), DatosCanal[canal].end());
	}
	// generar header de musica
	int TamanoFinal = GeneracionFinal.size();
	uint8_t cabecera[4] = {
		(uint8_t)(TamanoFinal & 0xFF),
		(uint8_t)((TamanoFinal >> 8) & 0xFF),
		(uint8_t)(InicioDatos & 0xFF),
		(uint8_t)((InicioDatos >> 8) & 0xFF)};
	GeneracionFinal.insert(GeneracionFinal.begin(), cabecera, cabecera + 4);
	// setear PunteroMusica (0x1FE) en el driver
	uint8_t DatosSubida[6] = {
		0x02,
		0x00,
		0xFE,
		0x01,
		(uint8_t)(InicioDatos & 0xFF),
		(uint8_t)((InicioDatos >> 8) & 0xFF)};
	GeneracionFinal.insert(GeneracionFinal.end(), DatosSubida, DatosSubida + 6);
	GeneracionFinal.push_back(0);
	GeneracionFinal.push_back(0);
	return InicioDatos + TamanoFinal;
}

void InstalarStubMusica() {
	// stub temporal, square wave 50% y un instrumento
	SonidoProyecto s;
	s.Nombre = "SquareWave";
	s.Datos = {
		0xC0,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0xC0,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0x44,
		0xC0,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xC3,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF};
	s.PuntoLoop = 0;
	SonidosProyecto.push_back(s);

	MusicaProyecto m;
	m.Nombre = "Musica1";
	InstrumentoMusica inst;
	inst.Nombre = "Inst1";
	inst.Sonido = 0;
	inst.EsGain = true;
	inst.A = 15;
	inst.D = 0;
	inst.S = 7;
	inst.R = 0;
	inst.Gain = 0x7F;
	inst.Frecuencia = 0x0300;
	m.Instrumentos.push_back(inst);
	m.NotasCanal[0].push_back({0, 0x2C, 0, 32});
	m.NotasCanal[0].push_back({0, 0x3C, 32, 32});
	m.NotasCanal[0].push_back({0, 0x4C, 64, 32});
	m.NotasCanal[0].push_back({0, 0x5C, 96, 32});
	MusicasProyecto.push_back(m);
}

void EnsamblarDatosMusicaSPC700(int Posicion, std::vector<BloqueDato> &Datos) {
	if(SonidosProyecto.empty() && MusicasProyecto.empty()) {
		InstalarStubMusica();
	}

	// ptrMusica es donde iran los datos de la musica cuando sea generada
	int ptrMusica = Posicion;

	// La tabla SRCN es una tabla conteniendo 2 pointers LE para cada sample
	std::vector<uint8_t> SRCN;
	std::vector<uint8_t> DatosSamples;
	for(size_t sample = 0; sample < SonidosProyecto.size(); sample++) {
		const SonidoProyecto &snd = SonidosProyecto[sample];
		if(snd.Datos.size() % 9 != 0) {
			throw std::runtime_error("Sonido " + snd.Nombre + " tiene datos BRR invalidos");
		}
		int comienzoSample = ptrMusica;
		int puntoLoop = ptrMusica + snd.PuntoLoop;
		SRCN.push_back(comienzoSample & 0xFF);
		SRCN.push_back((comienzoSample >> 8) & 0xFF);
		SRCN.push_back(puntoLoop & 0xFF);
		SRCN.push_back((puntoLoop >> 8) & 0xFF);
		DatosSamples.insert(DatosSamples.end(), snd.Datos.begin(), snd.Datos.end());
		ptrMusica += snd.Datos.size();
	}
	if(SRCN.size() > 128) {
		throw std::runtime_error("Demasiados sonidos, el SRCN supera el limite de 128 bytes");
	}
	// El SRCN va en 0x100
	uint8_t cabeceraSRCN[4] = {
		(uint8_t)(SRCN.size() & 0xFF),
		(uint8_t)((SRCN.size() >> 8) & 0xFF),
		0x00,
		0x01,
	};
	SRCN.insert(SRCN.begin(), cabeceraSRCN, cabeceraSRCN + 4);
	SRCN.push_back(0);
	SRCN.push_back(0);
	Datos.push_back({"DRIVER_AUDIO_SRCN", SRCN.data(), (uint32_t)SRCN.size()});
	// Los samples van en Posicion
	uint8_t cabeceraSamples[4] = {
		(uint8_t)(DatosSamples.size() & 0xFF),
		(uint8_t)((DatosSamples.size() >> 8) & 0xFF),
		(uint8_t)(Posicion & 0xFF),
		(uint8_t)((Posicion >> 8) & 0xFF),
	};
	DatosSamples.insert(DatosSamples.begin(), cabeceraSamples, cabeceraSamples + 4);
	DatosSamples.push_back(0);
	DatosSamples.push_back(0);
	Datos.push_back({"DRIVER_AUDIO_SAMPLES", DatosSamples.data(), (uint32_t)DatosSamples.size()});
	if(ptrMusica > 0xFFFF) {
		throw std::runtime_error("Los datos de sonido superan el limite de 64KB");
	}

	// ptrMusica ahora tiene la ubicacion de la musica (despues de samples)
	for(size_t musica = 0; musica < MusicasProyecto.size(); musica++) {
		MusicaProyecto &mus = MusicasProyecto[musica];
		mus.Ensamblar();
		int finalMus = mus.Generar(ptrMusica);
		if(finalMus > 0xFFFF) {
			throw std::runtime_error("Los datos de musica superan el limite de 64KB");
		}
		Datos.push_back({"MUSICA_" + mus.Nombre, mus.GeneracionFinal.data(), (uint32_t)mus.GeneracionFinal.size()});
	}

	// punteros de audio final
	cc.Etiqueta("PUNTEROS_AUDIO");
	cc.EscribirEtiqueta("DRIVER_AUDIO", true);
	cc.EscribirEtiqueta("DRIVER_AUDIO_SRCN", true);
	cc.EscribirEtiqueta("DRIVER_AUDIO_SAMPLES", true);
	for(size_t musica = 0; musica < MusicasProyecto.size(); musica++) {
		cc.EscribirEtiqueta("MUSICA_" + MusicasProyecto[musica].Nombre, true);
	}
}