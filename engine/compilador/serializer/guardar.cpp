#include "serializer.h"
#include "base64.h"
#include "spc700/spc700.h"
#include <cstdio>
#include <json.hpp>
#include <stdexcept>
#include <unordered_map>

using json = nlohmann::json;

static std::string IdDeBloque(size_t Indice) {
	char Buf[32];
	snprintf(Buf, sizeof(Buf), "bloque_id_%03u", (unsigned)(Indice + 1));
	return Buf;
}

static json SerializarParametroEspecial(const NodoBloque &Bloque, const std::vector<std::string> &Variables) {
	if((Bloque.TipoDeBloque == BLOQUE_VARIABLE || Bloque.TipoDeBloque == BLOQUE_VARIABLE_STORE) && Bloque.ParametroEspecial >= 0 &&
	   (size_t)Bloque.ParametroEspecial < Variables.size()) {
		return Variables[(size_t)Bloque.ParametroEspecial];
	}
	if(Bloque.TipoDeBloque == BLOQUE_EVENTO_CAMBIAR_ESCENA && Bloque.ParametroEspecial >= 0 &&
	   (size_t)Bloque.ParametroEspecial < EscenasProyecto.size()) {
		return EscenasProyecto[(size_t)Bloque.ParametroEspecial].Nombre;
	}
	return Bloque.ParametroEspecial;
}

static json SerializarBloqueAnidado(const NodoBloque &Bloque, const std::vector<std::string> &Variables) {
	json Nodo;
	Nodo["Operacion"] = ConvertirTipoDeBloqueAString(Bloque.TipoDeBloque);
	Nodo["ParametroEspecial"] = SerializarParametroEspecial(Bloque, Variables);
	json Entradas = json::array();
	for(const auto &Entrada : Bloque.Entradas) {
		Entradas.push_back(SerializarBloqueAnidado(Entrada, Variables));
	}
	Nodo["Entradas"] = Entradas;
	return Nodo;
}

static json SerializarEnlace(NodoBloque *Puntero, const std::unordered_map<const NodoBloque *, std::string> &Ids) {
	if(!Puntero) {
		return nullptr;
	}
	auto It = Ids.find(Puntero);
	if(It == Ids.end()) {
		return nullptr;
	}
	return It->second;
}

static json SerializarBloques(const std::vector<NodoBloque> &Bloques, const std::vector<std::string> &Variables) {
	std::unordered_map<const NodoBloque *, std::string> Ids;
	Ids.reserve(Bloques.size());
	for(size_t i = 0; i < Bloques.size(); i++) {
		Ids[&Bloques[i]] = IdDeBloque(i);
	}

	json Salida = json::object();
	for(size_t i = 0; i < Bloques.size(); i++) {
		json Nodo = SerializarBloqueAnidado(Bloques[i], Variables);
		Nodo["PosicionVisual"] = json::array({Bloques[i].PosicionVisualX, Bloques[i].PosicionVisualY});
		Nodo["Siguiente"] = SerializarEnlace(Bloques[i].Siguiente, Ids);
		Nodo["Previo"] = SerializarEnlace(Bloques[i].Previo, Ids);
		if(Bloques[i].Cuerpo) {
			Nodo["Cuerpo"] = SerializarEnlace(Bloques[i].Cuerpo, Ids);
		}
		if(Bloques[i].CuerpoSino) {
			Nodo["CuerpoSino"] = SerializarEnlace(Bloques[i].CuerpoSino, Ids);
		}
		Salida[Ids[&Bloques[i]]] = Nodo;
	}
	return Salida;
}

std::string ExportarProyectoAString() {
	json Escenas = json::object();
	for(const auto &EscenaActual : EscenasProyecto) {
		json Nodo;
		Nodo["GraficosPrincipales"] = CodificarBase64(EscenaActual.GraficosPrincipales, sizeof(EscenaActual.GraficosPrincipales));
		Nodo["GraficosHud"] = CodificarBase64(EscenaActual.GraficosHud, sizeof(EscenaActual.GraficosHud));
		Nodo["Tilemap1"] = CodificarBase64(EscenaActual.Tilemap1, sizeof(EscenaActual.Tilemap1));
		Nodo["Tilemap2"] = CodificarBase64(EscenaActual.Tilemap2, sizeof(EscenaActual.Tilemap2));
		Nodo["Tilemap3"] = CodificarBase64(EscenaActual.Tilemap3, sizeof(EscenaActual.Tilemap3));
		Nodo["Paleta"] = CodificarBase64(EscenaActual.Paleta, sizeof(EscenaActual.Paleta));
		Nodo["Variables"] = EscenaActual.Variables;
		Nodo["Bloques"] = SerializarBloques(EscenaActual.Bloques, EscenaActual.Variables);
		Escenas[EscenaActual.Nombre] = Nodo;
	}

	json Objetos = json::object();
	for(const auto &ObjetoActual : ObjetosProyecto) {
		json Nodo;
		Nodo["Variables"] = ObjetoActual.Variables;
		Nodo["Bloques"] = SerializarBloques(ObjetoActual.Bloques, ObjetoActual.Variables);
		Objetos[ObjetoActual.Nombre] = Nodo;
	}

	json Sonidos = json::object();
	for(const auto &Sonido : SonidosProyecto) {
		json Nodo;
		Nodo["Datos"] = CodificarBase64(Sonido.Datos.data(), Sonido.Datos.size());
		Nodo["PuntoLoop"] = Sonido.PuntoLoop;
		Sonidos[Sonido.Nombre] = Nodo;
	}

	json Musica = json::object();
	for(const auto &Mus : MusicasProyecto) {
		json Instrumentos = json::object();
		for(const auto &Inst : Mus.Instrumentos) {
			if(Inst.Sonido < 0 || (size_t)Inst.Sonido >= SonidosProyecto.size()) {
				throw std::runtime_error("instrumento sin sonido: " + Inst.Nombre);
			}
			json Item;
			Item["Sonido"] = SonidosProyecto[(size_t)Inst.Sonido].Nombre;
			Item["Frecuencia"] = Inst.Frecuencia;
			Item["EsGain"] = Inst.EsGain;
			Item["A"] = Inst.A;
			Item["D"] = Inst.D;
			Item["S"] = Inst.S;
			Item["R"] = Inst.R;
			Item["Gain"] = Inst.Gain;
			Instrumentos[Inst.Nombre] = Item;
		}
		json Notas = json::object();
		for(int Canal = 0; Canal < 8; Canal++) {
			if(Mus.NotasCanal[Canal].empty()) {
				continue;
			}
			json Lista = json::array();
			for(const auto &Nota : Mus.NotasCanal[Canal]) {
				if(Nota.Instrumento < 0 || (size_t)Nota.Instrumento >= Mus.Instrumentos.size()) {
					throw std::runtime_error("nota sin instrumento");
				}
				json Item;
				Item["Instrumento"] = Mus.Instrumentos[(size_t)Nota.Instrumento].Nombre;
				Item["Nota"] = Nota.Nota;
				Item["Posicion"] = Nota.Posicion;
				Item["Duracion"] = Nota.Duracion;
				Lista.push_back(std::move(Item));
			}
			Notas["Canal" + std::to_string(Canal + 1)] = std::move(Lista);
		}
		json Nodo;
		Nodo["Instrumentos"] = std::move(Instrumentos);
		Nodo["Notas"] = std::move(Notas);
		Musica[Mus.Nombre] = std::move(Nodo);
	}

	json Raiz;
	Raiz["Escenas"] = Escenas;
	Raiz["Objetos"] = Objetos;
	Raiz["Sonidos"] = Sonidos;
	Raiz["Musica"] = Musica;
	return Raiz.dump(1, '\t');
}
