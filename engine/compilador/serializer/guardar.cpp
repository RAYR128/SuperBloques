#include "serializer.h"
#include "base64.h"
#include <cstdio>
#include <json.hpp>
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

	json Raiz;
	Raiz["Escenas"] = Escenas;
	Raiz["Objetos"] = Objetos;
	return Raiz.dump(1, '\t');
}
