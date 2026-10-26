// Generacion de datos de musica para el SPC700
#include "asm/op.h"
#include "spc700.h"
#include <algorithm>
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

void MusicaProyecto::Ensamblar() {
	for(size_t canal = 0; canal < 8; canal++) {
		std::sort(NotasCanal[canal].begin(), NotasCanal[canal].end(), [](const NotaMusica &a, const NotaMusica &b) {
			return a.Posicion < b.Posicion;
		});
		DatosCanal[canal].clear();
		DatosCanal[canal].push_back(SB_ACMD_FINAL_DE_CANAL);
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
	m.NotasCanal[0].push_back({0, 0x3C, 0, 100});
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