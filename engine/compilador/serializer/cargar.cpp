#include "serializer.h"
#include "base64.h"
#include <cstring>
#include <json.hpp>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using json = nlohmann::json;

static int ResolverParametroEspecial(const json &Nodo, const std::vector<std::string> &Variables) {
	if(!Nodo.contains("ParametroEspecial")) {
		return 0;
	}
	const json &Parametro = Nodo["ParametroEspecial"];
	if(Parametro.is_number_integer()) {
		return Parametro.get<int>();
	}
	if(Parametro.is_string()) {
		const std::string Nombre = Parametro.get<std::string>();
		for(size_t i = 0; i < Variables.size(); i++) {
			if(Variables[i] == Nombre) {
				return (int)i;
			}
		}
		throw std::runtime_error("variable inexistente: " + Nombre);
	}
	throw std::runtime_error("ParametroEspecial debe ser entero o string");
}

static NodoBloque DeserializarBloqueAnidado(const json &Nodo, const std::vector<std::string> &Variables) {
	if(!Nodo.is_object()) {
		throw std::runtime_error("bloque debe ser un objeto");
	}
	if(!Nodo.contains("Operacion") || !Nodo["Operacion"].is_string()) {
		throw std::runtime_error("bloque sin Operacion");
	}

	NodoBloque Bloque;
	const std::string Operacion = Nodo["Operacion"].get<std::string>();
	Bloque.TipoDeBloque = ConvertirStringATipoDeBloque(Operacion);
	if(Bloque.TipoDeBloque == BLOQUE_MAX) {
		throw std::runtime_error("Operacion desconocida: " + Operacion);
	}

	Bloque.ParametroEspecial = ResolverParametroEspecial(Nodo, Variables);
	if(Nodo.contains("PosicionVisual") && Nodo["PosicionVisual"].is_array() && Nodo["PosicionVisual"].size() >= 2) {
		Bloque.PosicionVisualX = Nodo["PosicionVisual"][0].get<int>();
		Bloque.PosicionVisualY = Nodo["PosicionVisual"][1].get<int>();
	}
	if(Nodo.contains("Entradas")) {
		if(!Nodo["Entradas"].is_array()) {
			throw std::runtime_error("Entradas debe ser un array");
		}
		for(const auto &Entrada : Nodo["Entradas"]) {
			Bloque.Entradas.push_back(DeserializarBloqueAnidado(Entrada, Variables));
		}
	}
	return Bloque;
}

static NodoBloque *ResolverEnlace(const json &Nodo, const char *Campo, std::vector<NodoBloque> &Bloques, const std::unordered_map<std::string, size_t> &IdAIndice) {
	if(!Nodo.contains(Campo) || Nodo[Campo].is_null()) {
		return nullptr;
	}
	if(!Nodo[Campo].is_string()) {
		throw std::runtime_error(std::string(Campo) + " debe ser string o null");
	}
	const std::string Id = Nodo[Campo].get<std::string>();
	auto It = IdAIndice.find(Id);
	if(It == IdAIndice.end()) {
		throw std::runtime_error(std::string("id de bloque inexistente en ") + Campo + ": " + Id);
	}
	return &Bloques[It->second];
}

static void CargarBloques(const json &NodoBloques, std::vector<NodoBloque> &Salida, const std::vector<std::string> &Variables) {
	if(!NodoBloques.is_object()) {
		throw std::runtime_error("Bloques debe ser un objeto");
	}

	Salida.clear();
	Salida.reserve(NodoBloques.size());

	std::unordered_map<std::string, size_t> IdAIndice;
	std::vector<std::string> Ids;
	IdAIndice.reserve(NodoBloques.size());
	Ids.reserve(NodoBloques.size());

	for(auto It = NodoBloques.begin(); It != NodoBloques.end(); ++It) {
		const std::string &Id = It.key();
		if(IdAIndice.count(Id)) {
			throw std::runtime_error("id de bloque duplicado: " + Id);
		}
		IdAIndice[Id] = Salida.size();
		Ids.push_back(Id);
		Salida.push_back(DeserializarBloqueAnidado(It.value(), Variables));
	}

	for(size_t i = 0; i < Ids.size(); i++) {
		const json &Nodo = NodoBloques[Ids[i]];
		Salida[i].Siguiente = ResolverEnlace(Nodo, "Siguiente", Salida, IdAIndice);
		Salida[i].Previo = ResolverEnlace(Nodo, "Previo", Salida, IdAIndice);
	}
}

static void CargarBlob(const json &Nodo, const char *Clave, uint8_t *Destino, size_t Tamano) {
	std::memset(Destino, 0, Tamano);
	if(!Nodo.contains(Clave)) {
		return;
	}
	if(!Nodo[Clave].is_string()) {
		throw std::runtime_error(std::string(Clave) + " debe ser string base64");
	}
	if(!DecodificarBase64(Nodo[Clave].get<std::string>(), Destino, Tamano)) {
		throw std::runtime_error(std::string("base64 invalido o tamano incorrecto: ") + Clave);
	}
}

static void CargarVariables(const json &Nodo, std::vector<std::string> &Salida, size_t Maximo) {
	Salida.clear();
	if(!Nodo.contains("Variables")) {
		return;
	}
	if(!Nodo["Variables"].is_array()) {
		throw std::runtime_error("Variables debe ser un array");
	}
	if(Nodo["Variables"].size() > Maximo) {
		throw std::runtime_error("demasiadas variables (max " + std::to_string(Maximo) + ")");
	}

	std::unordered_set<std::string> Vistos;
	Vistos.reserve(Nodo["Variables"].size());
	for(const auto &Item : Nodo["Variables"]) {
		if(!Item.is_string()) {
			throw std::runtime_error("variable debe ser un string");
		}
		std::string Nombre = Item.get<std::string>();
		if(Nombre.empty()) {
			throw std::runtime_error("nombre de variable vacio");
		}
		if(!Vistos.insert(Nombre).second) {
			throw std::runtime_error("variable duplicada: " + Nombre);
		}
		Salida.push_back(std::move(Nombre));
	}
}

static const json &ObjetoOVacio(const json &Raiz, const char *Clave, json &Vacio) {
	if(!Raiz.contains(Clave)) {
		return Vacio;
	}
	if(!Raiz[Clave].is_object()) {
		throw std::runtime_error(std::string(Clave) + " debe ser un objeto");
	}
	return Raiz[Clave];
}

void CargarProyectoDesdeString(std::string JSONStr) {
	json Raiz = json::parse(JSONStr);
	json Vacio = json::object();
	const json &EscenasJson = ObjetoOVacio(Raiz, "Escenas", Vacio);
	const json &ObjetosJson = ObjetoOVacio(Raiz, "Objetos", Vacio);

	std::vector<Escena> NuevasEscenas;
	std::vector<ObjetoEscena> NuevosObjetos;
	NuevasEscenas.reserve(EscenasJson.size());
	NuevosObjetos.reserve(ObjetosJson.size());

	for(auto It = EscenasJson.begin(); It != EscenasJson.end(); ++It) {
		if(!It.value().is_object()) {
			throw std::runtime_error("escena debe ser un objeto");
		}
		Escena Actual;
		Actual.Nombre = It.key();
		CargarBlob(It.value(), "GraficosPrincipales", Actual.GraficosPrincipales, sizeof(Actual.GraficosPrincipales));
		CargarBlob(It.value(), "GraficosHud", Actual.GraficosHud, sizeof(Actual.GraficosHud));
		CargarBlob(It.value(), "Tilemap1", Actual.Tilemap1, sizeof(Actual.Tilemap1));
		CargarBlob(It.value(), "Tilemap2", Actual.Tilemap2, sizeof(Actual.Tilemap2));
		CargarBlob(It.value(), "Tilemap3", Actual.Tilemap3, sizeof(Actual.Tilemap3));
		CargarBlob(It.value(), "Paleta", Actual.Paleta, sizeof(Actual.Paleta));
		CargarVariables(It.value(), Actual.Variables, ESCENA_VARIABLES_MAX);
		if(It.value().contains("Bloques")) {
			CargarBloques(It.value()["Bloques"], Actual.Bloques, Actual.Variables);
		}
		NuevasEscenas.push_back(std::move(Actual));
	}

	for(auto It = ObjetosJson.begin(); It != ObjetosJson.end(); ++It) {
		if(!It.value().is_object()) {
			throw std::runtime_error("objeto debe ser un objeto");
		}
		ObjetoEscena Actual;
		Actual.Nombre = It.key();
		CargarVariables(It.value(), Actual.Variables, OBJETO_VARIABLES_MAX);
		if(It.value().contains("Bloques")) {
			CargarBloques(It.value()["Bloques"], Actual.Bloques, Actual.Variables);
		}
		NuevosObjetos.push_back(std::move(Actual));
	}

	EscenasProyecto = std::move(NuevasEscenas);
	ObjetosProyecto = std::move(NuevosObjetos);
}
