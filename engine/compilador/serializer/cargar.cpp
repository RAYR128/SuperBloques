#include "serializer.h"
#include "base64.h"
#include "spc700/spc700.h"
#include <cstring>
#include <json.hpp>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using json = nlohmann::json;

static int ResolverParametroEspecial(const json &Nodo, TipoBloque Tipo, const std::vector<std::string> &Variables, const std::vector<std::string> &NombresEscenas) {
	if(!Nodo.contains("ParametroEspecial")) {
		return 0;
	}
	const json &Parametro = Nodo["ParametroEspecial"];
	if(Parametro.is_number_integer()) {
		return Parametro.get<int>();
	}
	if(Parametro.is_string()) {
		const std::string Nombre = Parametro.get<std::string>();
		if(Tipo == BLOQUE_EVENTO_CAMBIAR_ESCENA) {
			for(size_t i = 0; i < NombresEscenas.size(); i++) {
				if(NombresEscenas[i] == Nombre) {
					return (int)i;
				}
			}
			throw std::runtime_error("escena inexistente: " + Nombre);
		}
		for(size_t i = 0; i < Variables.size(); i++) {
			if(Variables[i] == Nombre) {
				return (int)i;
			}
		}
		throw std::runtime_error("variable inexistente: " + Nombre);
	}
	throw std::runtime_error("ParametroEspecial debe ser entero o string");
}

static NodoBloque DeserializarBloqueAnidado(const json &Nodo, const std::vector<std::string> &Variables, const std::vector<std::string> &NombresEscenas) {
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

	Bloque.ParametroEspecial = ResolverParametroEspecial(Nodo, Bloque.TipoDeBloque, Variables, NombresEscenas);
	if(Nodo.contains("PosicionVisual") && Nodo["PosicionVisual"].is_array() && Nodo["PosicionVisual"].size() >= 2) {
		Bloque.PosicionVisualX = Nodo["PosicionVisual"][0].get<int>();
		Bloque.PosicionVisualY = Nodo["PosicionVisual"][1].get<int>();
	}
	if(Nodo.contains("Entradas")) {
		if(!Nodo["Entradas"].is_array()) {
			throw std::runtime_error("Entradas debe ser un array");
		}
		for(const auto &Entrada : Nodo["Entradas"]) {
			Bloque.Entradas.push_back(DeserializarBloqueAnidado(Entrada, Variables, NombresEscenas));
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

static void CargarBloques(const json &NodoBloques, std::vector<NodoBloque> &Salida, const std::vector<std::string> &Variables, const std::vector<std::string> &NombresEscenas) {
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
		Salida.push_back(DeserializarBloqueAnidado(It.value(), Variables, NombresEscenas));
	}

	for(size_t i = 0; i < Ids.size(); i++) {
		const json &Nodo = NodoBloques[Ids[i]];
		Salida[i].Siguiente = ResolverEnlace(Nodo, "Siguiente", Salida, IdAIndice);
		Salida[i].Previo = ResolverEnlace(Nodo, "Previo", Salida, IdAIndice);
		Salida[i].Cuerpo = ResolverEnlace(Nodo, "Cuerpo", Salida, IdAIndice);
		Salida[i].CuerpoSino = ResolverEnlace(Nodo, "CuerpoSino", Salida, IdAIndice);
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

static const json &ObjetoOVacio(const json &Raiz, const char *Clave, json &Vacio);

static const size_t SONIDOS_MAX = 32;
static const size_t BRR_DATOS_MAX = 32760;

static int EnteroJson(const json &Nodo, const char *Clave, int Min, int Max, int Defecto, bool Requerido) {
	if(!Nodo.contains(Clave)) {
		if(Requerido) {
			throw std::runtime_error(std::string("falta ") + Clave);
		}
		return Defecto;
	}
	const json &Valor = Nodo[Clave];
	if(!Valor.is_number_integer()) {
		throw std::runtime_error(std::string(Clave) + " fuera de rango");
	}
	const long long N = Valor.get<long long>();
	if(N < Min || N > Max) {
		throw std::runtime_error(std::string(Clave) + " fuera de rango");
	}
	return (int)N;
}

static bool BoolOpcional(const json &Nodo, const char *Clave, bool Defecto) {
	if(!Nodo.contains(Clave)) {
		return Defecto;
	}
	if(!Nodo[Clave].is_boolean()) {
		throw std::runtime_error(std::string(Clave) + " debe ser bool");
	}
	return Nodo[Clave].get<bool>();
}

static int IndicePorNombre(const std::vector<std::string> &Nombres, const std::string &Nombre) {
	for(size_t i = 0; i < Nombres.size(); i++) {
		if(Nombres[i] == Nombre) {
			return (int)i;
		}
	}
	return -1;
}

static int IndiceCanal(const std::string &Clave) {
	if(Clave.size() == 6 && Clave.compare(0, 5, "Canal") == 0 && Clave[5] >= '1' && Clave[5] <= '8') {
		return Clave[5] - '1';
	}
	return -1;
}

static std::vector<SonidoProyecto> CargarSonidos(const json &SonidosJson) {
	if(SonidosJson.size() > SONIDOS_MAX) {
		throw std::runtime_error("demasiados sonidos (max " + std::to_string(SONIDOS_MAX) + ")");
	}
	std::vector<SonidoProyecto> Salida;
	Salida.reserve(SonidosJson.size());
	for(auto It = SonidosJson.begin(); It != SonidosJson.end(); ++It) {
		if(It.key().empty()) {
			throw std::runtime_error("nombre de sonido vacio");
		}
		if(!It.value().is_object()) {
			throw std::runtime_error("sonido debe ser un objeto");
		}
		const json &Nodo = It.value();
		if(!Nodo.contains("Datos") || !Nodo["Datos"].is_string()) {
			throw std::runtime_error("Datos debe ser string base64");
		}
		SonidoProyecto Sonido;
		Sonido.Nombre = It.key();
		if(!DecodificarBase64(Nodo["Datos"].get<std::string>(), Sonido.Datos)) {
			throw std::runtime_error("base64 invalido: Datos");
		}
		if(Sonido.Datos.empty() || Sonido.Datos.size() % 9 != 0 || Sonido.Datos.size() > BRR_DATOS_MAX) {
			throw std::runtime_error("Datos BRR invalidos: " + Sonido.Nombre);
		}
		if(!Nodo.contains("PuntoLoop") || !Nodo["PuntoLoop"].is_number_integer()) {
			throw std::runtime_error("PuntoLoop debe ser entero");
		}
		const long long Loop = Nodo["PuntoLoop"].get<long long>();
		if(Loop < 0 || Loop >= (long long)Sonido.Datos.size() || Loop % 9 != 0) {
			throw std::runtime_error("PuntoLoop invalido: " + Sonido.Nombre);
		}
		Sonido.PuntoLoop = (uint16_t)Loop;
		Salida.push_back(std::move(Sonido));
	}
	return Salida;
}

static InstrumentoMusica CargarInstrumento(const std::string &Nombre, const json &Nodo, const std::vector<std::string> &NombresSonidos) {
	if(!Nodo.is_object()) {
		throw std::runtime_error("instrumento debe ser un objeto");
	}
	if(!Nodo.contains("Sonido") || !Nodo["Sonido"].is_string()) {
		throw std::runtime_error("sonido inexistente: ");
	}
	const std::string Sonido = Nodo["Sonido"].get<std::string>();
	const int Indice = IndicePorNombre(NombresSonidos, Sonido);
	if(Indice < 0) {
		throw std::runtime_error("sonido inexistente: " + Sonido);
	}
	InstrumentoMusica Inst;
	Inst.Nombre = Nombre;
	Inst.Sonido = Indice;
	Inst.Frecuencia = (uint16_t)EnteroJson(Nodo, "Frecuencia", 0, 65535, 0x0300, false);
	Inst.EsGain = BoolOpcional(Nodo, "EsGain", false);
	Inst.A = (uint8_t)EnteroJson(Nodo, "A", 0, 15, 0, false);
	Inst.D = (uint8_t)EnteroJson(Nodo, "D", 0, 7, 0, false);
	Inst.S = (uint8_t)EnteroJson(Nodo, "S", 0, 7, 0, false);
	Inst.R = (uint8_t)EnteroJson(Nodo, "R", 0, 31, 0, false);
	Inst.Gain = (uint8_t)EnteroJson(Nodo, "Gain", 0, 127, 127, false);
	return Inst;
}

static NotaMusica CargarNota(const json &Nodo, const std::vector<std::string> &NombresInstrumentos) {
	if(!Nodo.is_object()) {
		throw std::runtime_error("nota debe ser un objeto");
	}
	if(!Nodo.contains("Instrumento") || !Nodo["Instrumento"].is_string()) {
		throw std::runtime_error("instrumento inexistente: ");
	}
	const std::string Nombre = Nodo["Instrumento"].get<std::string>();
	const int Indice = IndicePorNombre(NombresInstrumentos, Nombre);
	if(Indice < 0) {
		throw std::runtime_error("instrumento inexistente: " + Nombre);
	}
	NotaMusica Nota;
	Nota.Instrumento = Indice;
	Nota.Nota = (uint8_t)EnteroJson(Nodo, "Nota", 0, 111, 0, true);
	Nota.Posicion = EnteroJson(Nodo, "Posicion", 0, 2147483647, 0, true);
	Nota.Duracion = EnteroJson(Nodo, "Duracion", 1, 2147483647, 1, true);
	return Nota;
}

static std::vector<MusicaProyecto> CargarMusicas(const json &MusicaJson, const std::vector<SonidoProyecto> &Sonidos) {
	std::vector<std::string> NombresSonidos;
	NombresSonidos.reserve(Sonidos.size());
	for(const auto &Sonido : Sonidos) {
		NombresSonidos.push_back(Sonido.Nombre);
	}

	std::vector<MusicaProyecto> Salida;
	Salida.reserve(MusicaJson.size());
	for(auto It = MusicaJson.begin(); It != MusicaJson.end(); ++It) {
		if(It.key().empty()) {
			throw std::runtime_error("nombre de musica vacio");
		}
		if(!It.value().is_object()) {
			throw std::runtime_error("musica debe ser un objeto");
		}
		json Vacio = json::object();
		const json &InstrumentosJson = ObjetoOVacio(It.value(), "Instrumentos", Vacio);
		const json &NotasJson = ObjetoOVacio(It.value(), "Notas", Vacio);

		MusicaProyecto Musica;
		Musica.Nombre = It.key();
		std::vector<std::string> NombresInstrumentos;
		NombresInstrumentos.reserve(InstrumentosJson.size());
		for(auto Inst = InstrumentosJson.begin(); Inst != InstrumentosJson.end(); ++Inst) {
			if(Inst.key().empty()) {
				throw std::runtime_error("nombre de instrumento vacio");
			}
			NombresInstrumentos.push_back(Inst.key());
			Musica.Instrumentos.push_back(CargarInstrumento(Inst.key(), Inst.value(), NombresSonidos));
		}
		for(auto Canal = NotasJson.begin(); Canal != NotasJson.end(); ++Canal) {
			const int Indice = IndiceCanal(Canal.key());
			if(Indice < 0) {
				throw std::runtime_error("canal desconocido: " + Canal.key());
			}
			if(!Canal.value().is_array()) {
				throw std::runtime_error(Canal.key() + " debe ser un array");
			}
			for(const auto &Nota : Canal.value()) {
				Musica.NotasCanal[Indice].push_back(CargarNota(Nota, NombresInstrumentos));
			}
		}
		Salida.push_back(std::move(Musica));
	}
	return Salida;
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

	std::vector<std::string> NombresEscenas;
	NombresEscenas.reserve(EscenasJson.size());
	for(auto It = EscenasJson.begin(); It != EscenasJson.end(); ++It) {
		NombresEscenas.push_back(It.key());
	}

	const json &SonidosJson = ObjetoOVacio(Raiz, "Sonidos", Vacio);
	const json &MusicaJson = ObjetoOVacio(Raiz, "Musica", Vacio);
	std::vector<SonidoProyecto> NuevosSonidos = CargarSonidos(SonidosJson);
	std::vector<MusicaProyecto> NuevasMusicas = CargarMusicas(MusicaJson, NuevosSonidos);

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
			CargarBloques(It.value()["Bloques"], Actual.Bloques, Actual.Variables, NombresEscenas);
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
			CargarBloques(It.value()["Bloques"], Actual.Bloques, Actual.Variables, NombresEscenas);
		}
		NuevosObjetos.push_back(std::move(Actual));
	}

	EscenasProyecto = std::move(NuevasEscenas);
	ObjetosProyecto = std::move(NuevosObjetos);
	SonidosProyecto = std::move(NuevosSonidos);
	MusicasProyecto = std::move(NuevasMusicas);
}
