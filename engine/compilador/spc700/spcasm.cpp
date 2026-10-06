// Ensamblador de codigo para SPC700
// https://snes.nesdev.org/wiki/SPC-700_instruction_set
#include "spc700.h"
#include "asm/op.h"

#include <cctype>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

struct Valor {
	bool usaEtiqueta = false;
	bool resuelto = true;
	int64_t n = 0;
};

// de acuerdo a los addressing modes de la CPU
enum class Clase {
	Reg,
	Imm,
	Mem,
	Idx,
	IndX,
	IndY,
	IndXInc,
	IndXP,
	IndPY
};

struct Operando {
	Clase clase = Clase::Mem;
	std::string reg;
	char indice = 0;
	bool tieneBit = false;
	int bit = 0;
	bool invertido = false;
	Valor valor;
	int ancho = 0;
};

struct Bloque {
	uint16_t aram = 0;
	std::vector<uint8_t> datos;
};

bool esIdentInicio(char c) {
	return std::isalpha((unsigned char)c) || c == '_';
}

bool esIdentCont(char c) {
	return std::isalnum((unsigned char)c) || c == '_';
}

std::string minusculas(std::string s) {
	for(char &c : s) {
		c = (char)std::tolower((unsigned char)c);
	}
	return s;
}

bool igual(const std::string &a, const char *b) {
	size_t n = 0;
	while(b[n] != '\0') {
		n++;
	}
	if(a.size() != n) {
		return false;
	}
	for(size_t i = 0; i < n; i++) {
		if(std::tolower((unsigned char)a[i]) != b[i]) {
			return false;
		}
	}
	return true;
}

bool terminaCon(const std::string &s, const char *sufijo) {
	size_t n = 0;
	while(sufijo[n] != '\0') {
		n++;
	}
	if(s.size() < n) {
		return false;
	}
	for(size_t i = 0; i < n; i++) {
		if(std::tolower((unsigned char)s[s.size() - n + i]) != sufijo[i]) {
			return false;
		}
	}
	return true;
}

std::string recortar(const std::string &s) {
	size_t a = 0;
	while(a < s.size() && std::isspace((unsigned char)s[a])) {
		a++;
	}
	size_t b = s.size();
	while(b > a && std::isspace((unsigned char)s[b - 1])) {
		b--;
	}
	return s.substr(a, b - a);
}

std::string sinEspacios(const std::string &s) {
	std::string r;
	r.reserve(s.size());
	for(char c : s) {
		if(!std::isspace((unsigned char)c)) {
			r.push_back(c);
		}
	}
	return r;
}

bool esRegistro(const std::string &s) {
	return igual(s, "a") || igual(s, "x") || igual(s, "y") || igual(s, "ya") || igual(s, "sp") || igual(s, "p") || igual(s, "c");
}

bool esMnemonicoDeBit(const std::string &s) {
	return s == "bbs" || s == "bbc" || s == "set" || s == "clr" || s == "or" || s == "and" || s == "eor" || s == "mov" || s == "not";
}

// https://snes.nesdev.org/wiki/SPC-700_instruction_set#Instructions
// utilizamos la sintaxis Intel para los ops enves de la 6502 ya que es mas familiar
bool mnemonicoConocido(const std::string &mn) {
	static const char *lista[] = {
		"nop",
		"brk",
		"clrp",
		"setp",
		"clrc",
		"ret",
		"reti",
		"setc",
		"ei",
		"di",
		"clrv",
		"notc",
		"sleep",
		"stop",
		"xcn",
		"asl",
		"lsr",
		"rol",
		"ror",
		"dec",
		"inc",
		"das",
		"daa",
		"push",
		"pop",
		"mul",
		"div",
		"jmp",
		"call",
		"tcall",
		"pcall",
		"decw",
		"incw",
		"bpl",
		"bra",
		"bmi",
		"bvc",
		"bvs",
		"bcc",
		"bcs",
		"bne",
		"beq",
		"mov",
		"cmp",
		"or",
		"and",
		"eor",
		"adc",
		"sbc",
		"tset",
		"tclr",
		"cmpw",
		"addw",
		"subw",
		"movw",
		"cbne",
		"dbnz",
		"bbs",
		"bbc",
		"set",
		"clr",
		"not"};
	for(const char *item : lista) {
		if(mn == item) {
			return true;
		}
	}
	return false;
}

class Ensamblador {
  public:
	std::vector<Bloque> ensamblar(const std::string &codigo) {
		correr(codigo, false);
		correr(codigo, true);
		return bloques;
	}

  private:
	std::map<std::string, std::string> defines;
	std::map<std::string, uint16_t> etiquetas;
	std::vector<Bloque> bloques;
	bool pasada2 = false;
	bool bloqueAbierto = false;
	bool usoAncho = false;
	int forzado = 0;
	int nLinea = 1;
	uint32_t pc = 0;
	uint32_t origenBloque = 0;

	[[noreturn]] void fallar(const std::string &texto) const {
		throw std::runtime_error("spc700:" + std::to_string(nLinea) + ": " + texto);
	}

	void correr(const std::string &codigo, bool segunda) {
		pasada2 = segunda;
		defines.clear();
		if(!segunda) {
			etiquetas.clear();
		}
		bloques.clear();
		bloqueAbierto = false;
		pc = 0;
		origenBloque = 0;
		nLinea = 1;
		for(size_t i = 0; i < codigo.size();) {
			size_t fin = i;
			while(fin < codigo.size() && codigo[fin] != '\n') {
				fin++;
			}
			std::string texto = codigo.substr(i, fin - i);
			if(!texto.empty() && texto.back() == '\r') {
				texto.pop_back();
			}
			procesarLinea(sinComentario(texto));
			i = (fin < codigo.size()) ? fin + 1 : fin;
			nLinea++;
		}
		if(pasada2 && !bloques.empty() && bloques.back().datos.empty()) {
			bloques.pop_back();
		}
	}

	static std::string sinComentario(const std::string &s) {
		bool comilla = false;
		for(size_t i = 0; i < s.size(); i++) {
			if(s[i] == '"') {
				comilla = !comilla;
			} else if(s[i] == ';' && !comilla) {
				return s.substr(0, i);
			}
		}
		return s;
	}

	void abrirBloque(uint16_t destino) {
		if(bloqueAbierto && pc == origenBloque) {
			origenBloque = destino;
			pc = destino;
			if(pasada2 && !bloques.empty()) {
				bloques.back().aram = destino;
			}
			return;
		}
		if(pasada2) {
			Bloque bloque;
			bloque.aram = destino;
			bloques.push_back(std::move(bloque));
		}
		bloqueAbierto = true;
		origenBloque = destino;
		pc = destino;
	}

	void emitir(uint8_t byte) {
		if(!bloqueAbierto) {
			fallar("falta base u org antes del codigo");
		}
		if(pc > 0xFFFF) {
			fallar("el PC de ARAM paso de $FFFF");
		}
		if(pc - origenBloque >= 65535) {
			fallar("el bloque supera 65535 bytes");
		}
		if(pasada2) {
			bloques.back().datos.push_back(byte);
		}
		pc++;
	}

	void definirEtiqueta(const std::string &nombre) {
		if(!bloqueAbierto) {
			fallar("etiqueta antes de base: " + nombre);
		}
		if(pc > 0xFFFF) {
			fallar("etiqueta fuera de ARAM: " + nombre);
		}
		uint16_t dir = (uint16_t)pc;
		if(!pasada2) {
			if(etiquetas.find(nombre) != etiquetas.end()) {
				fallar("etiqueta redefinida: " + nombre);
			}
			etiquetas[nombre] = dir;
		} else {
			auto it = etiquetas.find(nombre);
			if(it == etiquetas.end() || it->second != dir) {
				fallar("etiqueta inestable: " + nombre);
			}
		}
	}

	void saltar(const std::string &s, size_t &i) const {
		while(i < s.size() && std::isspace((unsigned char)s[i])) {
			i++;
		}
	}

	Valor combinar(const Valor &a, const Valor &b, int64_t resultado) const {
		Valor r;
		r.usaEtiqueta = a.usaEtiqueta || b.usaEtiqueta;
		r.resuelto = a.resuelto && b.resuelto;
		if(r.resuelto) {
			r.n = resultado;
		}
		return r;
	}

	Valor parseOr(const std::string &s, size_t &i);
	Valor parseXor(const std::string &s, size_t &i);
	Valor parseAnd(const std::string &s, size_t &i);
	Valor parseShift(const std::string &s, size_t &i);
	Valor parseSuma(const std::string &s, size_t &i);
	Valor parseMul(const std::string &s, size_t &i);
	Valor parseUnario(const std::string &s, size_t &i);
	Valor parsePrim(const std::string &s, size_t &i);

	int64_t leerDigitos(const std::string &s, size_t &i, int base, const char *vacio) {
		uint64_t v = 0;
		int digitos = 0;
		auto valorDigito = [](char c) -> int {
			if(c >= '0' && c <= '9') {
				return c - '0';
			}
			if(c >= 'a' && c <= 'f') {
				return c - 'a' + 10;
			}
			if(c >= 'A' && c <= 'F') {
				return c - 'A' + 10;
			}
			return -1;
		};
		while(i < s.size()) {
			int d = valorDigito(s[i]);
			if(d < 0 || d >= base) {
				break;
			}
			digitos++;
			if(v > ((uint64_t)INT64_MAX - (uint64_t)d) / (uint64_t)base) {
				fallar("numero demasiado grande");
			}
			v = v * (uint64_t)base + (uint64_t)d;
			i++;
		}
		if(digitos == 0) {
			fallar(vacio);
		}
		return (int64_t)v;
	}

	Valor evaluar(const std::string &texto) {
		std::string s = recortar(texto);
		if(s.empty()) {
			fallar("expresion vacia");
		}
		size_t i = 0;
		Valor v = parseOr(s, i);
		saltar(s, i);
		if(i != s.size()) {
			fallar("expresion invalida");
		}
		return v;
	}

	Valor exigirResuelto(const std::string &texto) {
		Valor v = evaluar(texto);
		if(!v.resuelto) {
			fallar("la direccion debe conocerse en la primera pasada");
		}
		return v;
	}

	void anchoVariable(Operando &op) {
		usoAncho = true;
		if(forzado == 1) {
			op.ancho = 1;
		} else if(forzado == 2 || op.valor.usaEtiqueta || !op.valor.resuelto) {
			op.ancho = 2;
		} else if(op.valor.n >= 0 && op.valor.n <= 0xFF) {
			op.ancho = 1;
		} else {
			op.ancho = 2;
		}
		if(op.valor.resuelto) {
			if(op.ancho == 1 && (op.valor.n < 0 || op.valor.n > 0xFF)) {
				fallar("no cabe en pagina directa");
			}
			if(op.ancho == 2 && (op.valor.n < 0 || op.valor.n > 0xFFFF)) {
				fallar("direccion fuera de rango");
			}
		}
	}

	void exigirDp(Operando &op) {
		usoAncho = true;
		if(forzado == 2) {
			fallar("no admite .w");
		}
		op.ancho = 1;
		if(op.valor.resuelto && (op.valor.n < 0 || op.valor.n > 0xFF)) {
			fallar("no cabe en pagina directa");
		}
	}

	void exigirAbs(Operando &op) {
		usoAncho = true;
		if(forzado == 1) {
			fallar("no admite .b");
		}
		op.ancho = 2;
		if(op.valor.resuelto && (op.valor.n < 0 || op.valor.n > 0xFFFF)) {
			fallar("direccion fuera de rango");
		}
	}

	void emitirDireccion(const Operando &op) {
		if(op.ancho != 1 && op.ancho != 2) {
			fallar("direccion invalida");
		}
		if(!op.valor.resuelto) {
			if(pasada2) {
				fallar("etiqueta no definida");
			}
			emitir(0);
			if(op.ancho == 2) {
				emitir(0);
			}
			return;
		}
		if(op.valor.n < 0 || (op.ancho == 1 && op.valor.n > 0xFF) || op.valor.n > 0xFFFF) {
			fallar("direccion fuera de rango");
		}
		emitir((uint8_t)op.valor.n);
		if(op.ancho == 2) {
			emitir((uint8_t)(op.valor.n >> 8));
		}
	}

	void emitirImm8(const Valor &v) {
		if(v.resuelto) {
			if(v.n < -128 || v.n > 255) {
				fallar("inmediato fuera de rango");
			}
		} else if(pasada2) {
			fallar("etiqueta no definida");
		}
		emitir((uint8_t)(v.resuelto ? v.n : 0));
	}

	// el byte relativo es el siguiente que se emite. destino - (pc + 1) equivale a destino - (pc_opcode + largo).
	void emitirRelativo(const Operando &op) {
		if(op.clase == Clase::Reg || op.clase == Clase::IndX || op.clase == Clase::IndY || op.clase == Clase::IndXInc || op.clase == Clase::IndXP || op.clase == Clase::IndPY) {
			fallar("destino de salto invalido");
		}
		bool crudo = op.valor.resuelto && !op.valor.usaEtiqueta && op.valor.n >= -128 && op.valor.n <= 255;
		if(crudo) {
			emitir((uint8_t)op.valor.n);
			return;
		}
		if(!op.valor.resuelto) {
			if(pasada2) {
				fallar("etiqueta no definida");
			}
			emitir(0);
			return;
		}
		int64_t rel = op.valor.n - (int64_t)(pc + 1);
		if(pasada2 && (rel < -128 || rel > 127)) {
			fallar("salto relativo fuera de rango");
		}
		emitir((uint8_t)rel);
	}

	void emitirDato(const Valor &v, int len) {
		if(!v.resuelto && pasada2) {
			fallar("etiqueta no definida");
		}
		uint64_t n = v.resuelto ? (uint64_t)v.n : 0;
		for(int i = 0; i < len; i++) {
			emitir((uint8_t)((n >> (8 * i)) & 0xFF));
		}
	}

	Operando parseOperando(const std::string &crudo) {
		std::string s = sinEspacios(crudo);
		if(s.empty()) {
			fallar("operando vacio");
		}
		Operando op;
		if(s[0] == '!' && (s.size() == 1 || !esIdentInicio(s[1]))) {
			op.invertido = true;
			s.erase(s.begin());
			if(s.empty()) {
				fallar("operando vacio");
			}
		}
		if(s.size() >= 2 && s[s.size() - 2] == '.' && s.back() >= '0' && s.back() <= '7') {
			op.tieneBit = true;
			op.bit = s.back() - '0';
			s.resize(s.size() - 2);
			if(s.empty()) {
				fallar("operando vacio");
			}
		}
		if(s[0] == '#') {
			op.clase = Clase::Imm;
			op.valor = evaluar(s.substr(1));
			return op;
		}
		if(s[0] == '(') {
			int nivel = 0;
			size_t cierra = std::string::npos;
			for(size_t i = 0; i < s.size(); i++) {
				if(s[i] == '(') {
					nivel++;
				} else if(s[i] == ')') {
					nivel--;
					if(nivel == 0) {
						cierra = i;
						break;
					}
				}
			}
			if(cierra == std::string::npos) {
				fallar("parentesis sin cerrar");
			}
			std::string dentro = s.substr(1, cierra - 1);
			std::string despues = s.substr(cierra + 1);
			if(igual(dentro, "x") && despues.empty()) {
				op.clase = Clase::IndX;
				return op;
			}
			if(igual(dentro, "y") && despues.empty()) {
				op.clase = Clase::IndY;
				return op;
			}
			if(igual(dentro, "x+") && despues.empty()) {
				op.clase = Clase::IndXInc;
				return op;
			}
			if(igual(dentro, "x") && despues == "+") {
				fallar("usa (x+) en lugar de (x)+");
			}
			if(igual(despues, "+y")) {
				op.clase = Clase::IndPY;
				op.valor = evaluar(dentro);
				return op;
			}
			if(despues.empty() && terminaCon(dentro, "+x")) {
				op.clase = Clase::IndXP;
				op.valor = evaluar(dentro.substr(0, dentro.size() - 2));
				return op;
			}
			if(!despues.empty()) {
				fallar("forma de operando no valida");
			}
			op.clase = Clase::Mem;
			op.valor = evaluar(dentro);
			return op;
		}
		if(terminaCon(s, "+x")) {
			op.clase = Clase::Idx;
			op.indice = 'x';
			op.valor = evaluar(s.substr(0, s.size() - 2));
			return op;
		}
		if(terminaCon(s, "+y")) {
			op.clase = Clase::Idx;
			op.indice = 'y';
			op.valor = evaluar(s.substr(0, s.size() - 2));
			return op;
		}
		if(esRegistro(s)) {
			op.clase = Clase::Reg;
			op.reg = minusculas(s);
			return op;
		}
		op.clase = Clase::Mem;
		op.valor = evaluar(s);
		return op;
	}

	std::vector<std::string> partirComas(const std::string &s) const {
		std::vector<std::string> partes;
		std::string actual;
		int paren = 0;
		bool comilla = false;
		for(size_t i = 0; i < s.size(); i++) {
			char c = s[i];
			if(comilla) {
				actual.push_back(c);
				if(c == '\\' && i + 1 < s.size()) {
					actual.push_back(s[++i]);
					continue;
				}
				if(c == '"') {
					comilla = false;
				}
				continue;
			}
			if(c == '"') {
				comilla = true;
				actual.push_back(c);
				continue;
			}
			if(c == '(') {
				paren++;
				actual.push_back(c);
				continue;
			}
			if(c == ')') {
				if(paren > 0) {
					paren--;
				}
				actual.push_back(c);
				continue;
			}
			if(c == ',' && paren == 0) {
				partes.push_back(recortar(actual));
				actual.clear();
				continue;
			}
			actual.push_back(c);
		}
		if(!actual.empty() || !partes.empty()) {
			partes.push_back(recortar(actual));
		}
		return partes;
	}

	std::string expandir(const std::string &entrada, int profundidad) {
		if(profundidad > 32) {
			fallar("recursion de defines");
		}
		std::string salida;
		for(size_t i = 0; i < entrada.size();) {
			if(entrada[i] == '\\' && i + 1 < entrada.size() && entrada[i + 1] == '!') {
				salida.push_back('!');
				i += 2;
				continue;
			}
			if(entrada[i] == '"') {
				salida.push_back('"');
				i++;
				while(i < entrada.size() && entrada[i] != '"') {
					salida.push_back(entrada[i]);
					i++;
				}
				if(i < entrada.size()) {
					salida.push_back('"');
					i++;
				}
				continue;
			}
			if(entrada[i] == '!') {
				size_t j = i + 1;
				if(j < entrada.size() && esIdentInicio(entrada[j])) {
					size_t k = j + 1;
					while(k < entrada.size() && esIdentCont(entrada[k])) {
						k++;
					}
					std::string nombre = entrada.substr(j, k - j);
					auto it = defines.find(nombre);
					if(it == defines.end()) {
						fallar("define no definido: !" + nombre);
					}
					salida += expandir(it->second, profundidad + 1);
					i = k;
					continue;
				}
			}
			salida.push_back(entrada[i]);
			i++;
		}
		return salida;
	}

	void definirDefine(const std::string &nombre, const std::string &valor) {
		if(nombre.empty() || !esIdentInicio(nombre[0])) {
			fallar("nombre de define invalido");
		}
		for(char c : nombre) {
			if(!esIdentCont(c)) {
				fallar("nombre de define invalido");
			}
		}
		defines[nombre] = recortar(valor);
	}

	void procesarLinea(const std::string &linea) {
		size_t i = 0;
		while(i < linea.size()) {
			saltar(linea, i);
			if(i >= linea.size()) {
				return;
			}
			if(linea[i] == ':') {
				i++;
				continue;
			}
			if(esIdentInicio(linea[i])) {
				size_t k = i + 1;
				while(k < linea.size() && esIdentCont(linea[k])) {
					k++;
				}
				if(k < linea.size() && linea[k] == ':') {
					definirEtiqueta(linea.substr(i, k - i));
					i = k + 1;
					continue;
				}
			}
			if(linea[i] == '!') {
				size_t j = i + 1;
				if(j < linea.size() && esIdentInicio(linea[j])) {
					size_t k = j + 1;
					while(k < linea.size() && esIdentCont(linea[k])) {
						k++;
					}
					size_t t = k;
					saltar(linea, t);
					bool asigna = t < linea.size() && linea[t] == '=';
					bool otraAsignacion = t + 1 < linea.size() && linea[t + 1] == '=' &&
										  (linea[t] == '+' || linea[t] == ':' || linea[t] == '#' || linea[t] == '?');
					if(otraAsignacion || (asigna && t + 1 < linea.size() && linea[t + 1] == '=')) {
						fallar("asignacion de define no soportada");
					}
					if(asigna) {
						definirDefine(linea.substr(j, k - j), linea.substr(t + 1));
						return;
					}
				}
			}
			std::string sentencia = leerSentencia(linea, i);
			if(i < linea.size() && linea[i] == ':') {
				i++;
			}
			procesarSentencia(expandir(recortar(sentencia), 0));
		}
	}

	std::string leerSentencia(const std::string &s, size_t &i) const {
		size_t inicio = i;
		int paren = 0;
		bool comilla = false;
		for(; i < s.size(); i++) {
			char c = s[i];
			if(comilla) {
				if(c == '"') {
					comilla = false;
				}
				continue;
			}
			if(c == '"') {
				comilla = true;
				continue;
			}
			if(c == '(') {
				paren++;
				continue;
			}
			if(c == ')') {
				if(paren > 0) {
					paren--;
				}
				continue;
			}
			if(c == ':' && paren == 0) {
				break;
			}
		}
		return s.substr(inicio, i - inicio);
	}

	void procesarSentencia(const std::string &sentenciaCruda) {
		std::string sentencia = recortar(sentenciaCruda);
		if(sentencia.empty()) {
			return;
		}
		size_t i = 0;
		while(i < sentencia.size() && !std::isspace((unsigned char)sentencia[i])) {
			i++;
		}
		std::string mn = minusculas(sentencia.substr(0, i));
		std::string resto = recortar(sentencia.substr(i));
		forzado = 0;
		auto punto = mn.rfind('.');
		if(punto != std::string::npos) {
			std::string sufijo = mn.substr(punto + 1);
			if(sufijo == "b") {
				forzado = 1;
			} else if(sufijo == "w") {
				forzado = 2;
			} else {
				fallar("sufijo invalido");
			}
			mn = mn.substr(0, punto);
		}
		if(mn == "base" || mn == "org" || mn == "skip" || mn == "db" || mn == "dw" || mn == "dl" || mn == "dd") {
			if(forzado != 0) {
				fallar("sufijo no aplica");
			}
			if(mn == "base" || mn == "org") {
				if(igual(sinEspacios(resto), "off")) {
					fallar("off no esta soportado");
				}
				Valor dir = exigirResuelto(resto);
				if(dir.n < 0 || dir.n > 0xFFFF) {
					fallar("direccion fuera de rango");
				}
				abrirBloque((uint16_t)dir.n);
				return;
			}
			if(mn == "skip") {
				if(!bloqueAbierto) {
					fallar("falta base u org antes del codigo");
				}
				Valor n = exigirResuelto(resto);
				if(n.n < 0) {
					fallar("skip negativo");
				}
				uint64_t destino = (uint64_t)pc + (uint64_t)n.n;
				if(destino > 0xFFFF) {
					fallar("direccion fuera de rango");
				}
				abrirBloque((uint16_t)destino);
				return;
			}
			int len = (mn == "db") ? 1 : (mn == "dw") ? 2
									 : (mn == "dl")	  ? 3
													  : 4;
			emitirDatos(resto, len);
			return;
		}
		codificar(mn, resto);
	}

	void emitirDatos(const std::string &resto, int len) {
		auto partes = partirComas(resto);
		if(partes.empty()) {
			fallar("dato vacio");
		}
		for(const std::string &parte : partes) {
			if(parte.empty()) {
				fallar("dato vacio");
			}
			if(parte[0] == '"') {
				if(parte.size() < 2 || parte.back() != '"') {
					fallar("string sin cerrar");
				}
				std::string texto = parte.substr(1, parte.size() - 2);
				for(size_t i = 0; i < texto.size(); i++) {
					char c = texto[i];
					if(c == '\\' && i + 1 < texto.size()) {
						i++;
						c = texto[i];
						if(c == 'n') {
							c = '\n';
						} else if(c == 't') {
							c = '\t';
						} else if(c == '0') {
							c = '\0';
						}
					}
					uint64_t n = (unsigned char)c;
					for(int b = 0; b < len; b++) {
						emitir((uint8_t)((n >> (8 * b)) & 0xFF));
					}
				}
				continue;
			}
			std::string expr = parte;
			if(!expr.empty() && expr[0] == '#') {
				expr.erase(expr.begin());
			}
			emitirDato(evaluar(expr), len);
		}
	}

	void codificar(std::string mn, const std::string &resto) {
		std::vector<Operando> ops;
		if(!recortar(resto).empty()) {
			for(const std::string &parte : partirComas(resto)) {
				ops.push_back(parseOperando(parte));
			}
		}
		int bit = -1;
		bool esBit = false;
		if(!ops.empty() && ops[0].tieneBit) {
			bit = ops[0].bit;
			esBit = true;
		} else if(ops.size() >= 2 && ops[1].tieneBit) {
			bit = ops[1].bit;
			esBit = true;
		}
		if(esBit) {
			// Asar, con .bit en el operando, solo recorta el '1' final del mnemonico.
			if(!mn.empty() && mn.back() == '1') {
				mn.pop_back();
			}
		} else if(!mn.empty() && mn.back() >= '0' && mn.back() <= '9') {
			std::string raiz = mn.substr(0, mn.size() - 1);
			if(esMnemonicoDeBit(raiz)) {
				bit = mn.back() - '0';
				if(bit > 7) {
					fallar("bit fuera de rango");
				}
				mn = raiz;
				esBit = true;
			}
		}
		if(mn == "tset1") {
			mn = "tset";
		} else if(mn == "tclr1") {
			mn = "tclr";
		}
		usoAncho = false;
		if(esBit) {
			if(!codificarBit(mn, ops, bit)) {
				fallar("forma de operando no valida");
			}
			if(forzado != 0 && !usoAncho) {
				fallar("sufijo no aplica");
			}
			return;
		}
		bool listo = false;
		if(ops.empty()) {
			listo = codificarImplicito(mn);
		} else if(ops.size() == 1) {
			listo = codificarUno(mn, ops[0]);
		} else if(ops.size() == 2) {
			listo = codificarDos(mn, ops[0], ops[1]);
		} else {
			fallar("demasiados operandos");
		}
		if(!listo) {
			if(!mnemonicoConocido(mn)) {
				fallar("mnemonico desconocido: " + mn);
			}
			fallar("forma de operando no valida");
		}
		if(forzado != 0 && !usoAncho) {
			fallar("sufijo no aplica");
		}
	}

	bool codificarImplicito(const std::string &mn) {
		struct Item {
			const char *nombre;
			uint8_t op;
		};
		static const Item tabla[] = {
			{"nop",	0x00},
			{"brk",	0x0F},
			{"clrp",	 0x20},
			{"setp",	 0x40},
			{"clrc",	 0x60},
			{"ret",	0x6F},
			{"reti",	 0x7F},
			{"setc",	 0x80},
			{"ei",	   0xA0},
			{"di",	   0xC0},
			{"clrv",	 0xE0},
			{"notc",	 0xED},
			{"sleep", 0xEF},
			{"stop",	 0xFF},
			{"xcn",	0x9F},
		};
		for(const Item &item : tabla) {
			if(mn == item.nombre) {
				emitir(item.op);
				return true;
			}
		}
		return false;
	}

	bool codificarRegistro(const std::string &mn, const Operando &op) {
		if(op.clase != Clase::Reg) {
			return false;
		}
		struct Item {
			const char *nombre;
			const char *reg;
			uint8_t codigo;
		};
		static const Item tabla[] = {
			{"asl",	"a",	 0x1C},
			{"lsr",	"a",	 0x5C},
			{"rol",	"a",	 0x3C},
			{"ror",	"a",	 0x7C},
			{"dec",	"a",	 0x9C},
			{"inc",	"a",	 0xBC},
			{"das",	"a",	 0xBE},
			{"daa",	"a",	 0xDF},
			{"push", "a",  0x2D},
			{"pop",	"a",	 0xAE},
			{"xcn",	"a",	 0x9F},
			{"dec",	"x",	 0x1D},
			{"inc",	"x",	 0x3D},
			{"push", "x",  0x4D},
			{"pop",	"x",	 0xCE},
			{"dec",	"y",	 0xDC},
			{"inc",	"y",	 0xFC},
			{"push", "y",  0x6D},
			{"pop",	"y",	 0xEE},
			{"push", "p",  0x0D},
			{"pop",	"p",	 0x8E},
			{"mul",	"ya", 0xCF},
		};
		for(const Item &item : tabla) {
			if(mn == item.nombre && op.reg == item.reg) {
				emitir(item.codigo);
				return true;
			}
		}
		return false;
	}

	bool codificarUno(const std::string &mn, Operando op) {
		if(codificarRegistro(mn, op)) {
			return true;
		}
		if(op.clase == Clase::Idx && op.indice == 'x') {
			uint8_t codigo = 0;
			if(mn == "asl") {
				codigo = 0x1B;
			} else if(mn == "dec") {
				codigo = 0x9B;
			} else if(mn == "inc") {
				codigo = 0xBB;
			} else if(mn == "lsr") {
				codigo = 0x5B;
			} else if(mn == "rol") {
				codigo = 0x3B;
			} else if(mn == "ror") {
				codigo = 0x7B;
			} else {
				return false;
			}
			exigirDp(op);
			emitir(codigo);
			emitirDireccion(op);
			return true;
		}
		if(op.clase == Clase::Mem || op.clase == Clase::Imm) {
			uint8_t dp = 0;
			uint8_t abs = 0;
			bool variable = false;
			if(mn == "asl") {
				dp = 0x0B;
				abs = 0x0C;
				variable = true;
			} else if(mn == "dec") {
				dp = 0x8B;
				abs = 0x8C;
				variable = true;
			} else if(mn == "inc") {
				dp = 0xAB;
				abs = 0xAC;
				variable = true;
			} else if(mn == "lsr") {
				dp = 0x4B;
				abs = 0x4C;
				variable = true;
			} else if(mn == "rol") {
				dp = 0x2B;
				abs = 0x2C;
				variable = true;
			} else if(mn == "ror") {
				dp = 0x6B;
				abs = 0x6C;
				variable = true;
			}
			if(variable) {
				if(op.clase == Clase::Imm) {
					op.clase = Clase::Mem;
				}
				anchoVariable(op);
				emitir(op.ancho == 1 ? dp : abs);
				emitirDireccion(op);
				return true;
			}
		}
		if(mn == "jmp" && op.clase == Clase::IndXP) {
			exigirAbs(op);
			emitir(0x1F);
			emitirDireccion(op);
			return true;
		}
		if((mn == "jmp" || mn == "call") && (op.clase == Clase::Mem || op.clase == Clase::Imm)) {
			exigirAbs(op);
			emitir(mn == "jmp" ? 0x5F : 0x3F);
			emitirDireccion(op);
			return true;
		}
		if(mn == "tcall" && (op.clase == Clase::Mem || op.clase == Clase::Imm)) {
			if(op.valor.resuelto && (op.valor.n < 0 || op.valor.n > 15)) {
				fallar("tcall fuera de rango");
			}
			if(pasada2 && !op.valor.resuelto) {
				fallar("etiqueta no definida");
			}
			uint8_t n = op.valor.resuelto ? (uint8_t)op.valor.n : 0;
			emitir((uint8_t)((n << 4) | 0x01));
			return true;
		}
		if(mn == "pcall" && (op.clase == Clase::Mem || op.clase == Clase::Imm)) {
			emitir(0x4F);
			emitirImm8(op.valor);
			return true;
		}
		if((mn == "decw" || mn == "incw") && (op.clase == Clase::Mem || op.clase == Clase::Imm)) {
			exigirDp(op);
			emitir(mn == "decw" ? 0x1A : 0x3A);
			emitirDireccion(op);
			return true;
		}
		uint8_t salto = 0;
		if(mn == "bpl") {
			salto = 0x10;
		} else if(mn == "bra") {
			salto = 0x2F;
		} else if(mn == "bmi") {
			salto = 0x30;
		} else if(mn == "bvc") {
			salto = 0x50;
		} else if(mn == "bvs") {
			salto = 0x70;
		} else if(mn == "bcc") {
			salto = 0x90;
		} else if(mn == "bcs") {
			salto = 0xB0;
		} else if(mn == "bne") {
			salto = 0xD0;
		} else if(mn == "beq") {
			salto = 0xF0;
		}
		if(salto != 0 && (op.clase == Clase::Mem || op.clase == Clase::Imm)) {
			emitir(salto);
			emitirRelativo(op);
			return true;
		}
		return false;
	}

	bool codificarBit(const std::string &mn, std::vector<Operando> ops, int bit) {
		if(bit < 0 || bit > 7) {
			fallar("bit fuera de rango");
		}
		if(mn == "set" || mn == "clr") {
			if(ops.size() != 1 || (ops[0].clase != Clase::Mem && ops[0].clase != Clase::Imm) || ops[0].invertido) {
				return false;
			}
			exigirDp(ops[0]);
			emitir((uint8_t)((mn == "set" ? 0x02 : 0x12) | (bit << 5)));
			emitirDireccion(ops[0]);
			return true;
		}
		if(mn == "bbs" || mn == "bbc") {
			if(ops.size() != 2 || (ops[0].clase != Clase::Mem && ops[0].clase != Clase::Imm) || ops[0].invertido) {
				return false;
			}
			exigirDp(ops[0]);
			emitir((uint8_t)((mn == "bbs" ? 0x03 : 0x13) | (bit << 5)));
			emitirDireccion(ops[0]);
			emitirRelativo(ops[1]);
			return true;
		}
		if(mn == "or" || mn == "and" || mn == "eor" || mn == "mov" || mn == "not") {
			const Operando *dir = nullptr;
			bool movACarry = false;
			if(ops.size() == 1 && (ops[0].clase == Clase::Mem || ops[0].clase == Clase::Imm)) {
				dir = &ops[0];
			} else if(ops.size() == 2 && ops[0].clase == Clase::Reg && ops[0].reg == "c" && (ops[1].clase == Clase::Mem || ops[1].clase == Clase::Imm)) {
				dir = &ops[1];
			} else if(mn == "mov" && ops.size() == 2 && (ops[0].clase == Clase::Mem || ops[0].clase == Clase::Imm) && ops[1].clase == Clase::Reg && ops[1].reg == "c") {
				dir = &ops[0];
				movACarry = true;
			} else {
				return false;
			}
			bool invertir = dir->invertido;
			if(invertir && mn != "or" && mn != "and") {
				return false;
			}
			if(dir->valor.resuelto && (dir->valor.n < 0 || dir->valor.n >= 0x2000)) {
				fallar("direccion de bit fuera de rango");
			}
			if(pasada2 && !dir->valor.resuelto) {
				fallar("etiqueta no definida");
			}
			uint8_t codigo = 0xEA;
			if(movACarry) {
				codigo = 0xCA;
			} else if(invertir && mn == "or") {
				codigo = 0x2A;
			} else if(invertir && mn == "and") {
				codigo = 0x6A;
			} else if(mn == "or") {
				codigo = 0x0A;
			} else if(mn == "and") {
				codigo = 0x4A;
			} else if(mn == "eor") {
				codigo = 0x8A;
			} else if(mn == "mov") {
				codigo = 0xAA;
			}
			emitir(codigo);
			uint16_t empaquetado = 0;
			if(dir->valor.resuelto) {
				empaquetado = (uint16_t)((bit << 13) | (dir->valor.n & 0x1FFF));
			}
			emitir((uint8_t)(empaquetado & 0xFF));
			emitir((uint8_t)(empaquetado >> 8));
			return true;
		}
		return false;
	}

	bool codificarDos(const std::string &mn, Operando a, Operando b) {
		if(mn == "mov" && codificarMov(a, b)) {
			return true;
		}
		if(mn == "cmp" && a.clase == Clase::Reg && (a.reg == "x" || a.reg == "y") && (b.clase == Clase::Mem || b.clase == Clase::Imm)) {
			if(b.clase == Clase::Imm) {
				emitir(a.reg == "x" ? 0xC8 : 0xAD);
				emitirImm8(b.valor);
				return true;
			}
			anchoVariable(b);
			if(a.reg == "x") {
				emitir(b.ancho == 1 ? 0x3E : 0x1E);
			} else {
				emitir(b.ancho == 1 ? 0x7E : 0x5E);
			}
			emitirDireccion(b);
			return true;
		}
		int off = -1;
		if(mn == "or") {
			off = 0x00;
		} else if(mn == "and") {
			off = 0x20;
		} else if(mn == "eor") {
			off = 0x40;
		} else if(mn == "cmp") {
			off = 0x60;
		} else if(mn == "adc") {
			off = 0x80;
		} else if(mn == "sbc") {
			off = 0xA0;
		}
		if(off >= 0 && codificarAlu(off, a, b)) {
			return true;
		}
		if((mn == "tset" || mn == "tclr") && (a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Reg && b.reg == "a") {
			exigirAbs(a);
			emitir(mn == "tset" ? 0x0E : 0x4E);
			emitirDireccion(a);
			return true;
		}
		if(mn == "div" && a.clase == Clase::Reg && a.reg == "ya" && b.clase == Clase::Reg && b.reg == "x") {
			emitir(0x9E);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "ya" && (b.clase == Clase::Mem || b.clase == Clase::Imm)) {
			uint8_t codigo = 0;
			if(mn == "cmpw") {
				codigo = 0x5A;
			} else if(mn == "addw") {
				codigo = 0x7A;
			} else if(mn == "subw") {
				codigo = 0x9A;
			} else if(mn == "movw") {
				codigo = 0xBA;
			}
			if(codigo != 0) {
				exigirDp(b);
				emitir(codigo);
				emitirDireccion(b);
				return true;
			}
		}
		if(mn == "movw" && (a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Reg && b.reg == "ya") {
			exigirDp(a);
			emitir(0xDA);
			emitirDireccion(a);
			return true;
		}
		if(mn == "cbne" && a.clase == Clase::Idx && a.indice == 'x') {
			exigirDp(a);
			emitir(0xDE);
			emitirDireccion(a);
			emitirRelativo(b);
			return true;
		}
		if(mn == "dbnz" && a.clase == Clase::Reg && a.reg == "y") {
			emitir(0xFE);
			emitirRelativo(b);
			return true;
		}
		if((mn == "dbnz" || mn == "cbne") && (a.clase == Clase::Mem || a.clase == Clase::Imm)) {
			exigirDp(a);
			emitir(mn == "dbnz" ? 0x6E : 0x2E);
			emitirDireccion(a);
			emitirRelativo(b);
			return true;
		}
		return false;
	}

	bool codificarMov(Operando a, Operando b) {
		if(a.clase == Clase::IndXInc && b.clase == Clase::Reg && b.reg == "a") {
			emitir(0xAF);
			return true;
		}
		if(a.clase == Clase::IndX && b.clase == Clase::Reg && b.reg == "a") {
			emitir(0xC6);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndXInc) {
			emitir(0xBF);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndX) {
			emitir(0xE6);
			return true;
		}
		if(a.clase == Clase::Reg && b.clase == Clase::Reg) {
			uint8_t codigo = 0;
			if(a.reg == "a" && b.reg == "x") {
				codigo = 0x7D;
			} else if(a.reg == "a" && b.reg == "y") {
				codigo = 0xDD;
			} else if(a.reg == "x" && b.reg == "a") {
				codigo = 0x5D;
			} else if(a.reg == "x" && b.reg == "sp") {
				codigo = 0x9D;
			} else if(a.reg == "y" && b.reg == "a") {
				codigo = 0xFD;
			} else if(a.reg == "sp" && b.reg == "x") {
				codigo = 0xBD;
			} else {
				return false;
			}
			emitir(codigo);
			return true;
		}
		if(a.clase == Clase::IndXP && b.clase == Clase::Reg && b.reg == "a") {
			exigirDp(a);
			emitir(0xC7);
			emitirDireccion(a);
			return true;
		}
		if(a.clase == Clase::IndPY && b.clase == Clase::Reg && b.reg == "a") {
			exigirDp(a);
			emitir(0xD7);
			emitirDireccion(a);
			return true;
		}
		if(a.clase == Clase::Idx && b.clase == Clase::Reg && b.reg == "a" && a.indice == 'x') {
			anchoVariable(a);
			emitir(a.ancho == 1 ? 0xD4 : 0xD5);
			emitirDireccion(a);
			return true;
		}
		if(a.clase == Clase::Idx && b.clase == Clase::Reg && b.reg == "a" && a.indice == 'y') {
			exigirAbs(a);
			emitir(0xD6);
			emitirDireccion(a);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Reg && b.reg == "a") {
			anchoVariable(a);
			emitir(a.ancho == 1 ? 0xC4 : 0xC5);
			emitirDireccion(a);
			return true;
		}
		if(a.clase == Clase::Idx && b.clase == Clase::Reg && b.reg == "y" && a.indice == 'x') {
			exigirDp(a);
			emitir(0xDB);
			emitirDireccion(a);
			return true;
		}
		if(a.clase == Clase::Idx && b.clase == Clase::Reg && b.reg == "x" && a.indice == 'y') {
			exigirDp(a);
			emitir(0xD9);
			emitirDireccion(a);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Reg && b.reg == "x") {
			anchoVariable(a);
			emitir(a.ancho == 1 ? 0xD8 : 0xC9);
			emitirDireccion(a);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Reg && b.reg == "y") {
			anchoVariable(a);
			emitir(a.ancho == 1 ? 0xCB : 0xCC);
			emitirDireccion(a);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Imm) {
			emitir(0xE8);
			emitirImm8(b.valor);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndXP) {
			exigirDp(b);
			emitir(0xE7);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndPY) {
			exigirDp(b);
			emitir(0xF7);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Idx && b.indice == 'x') {
			anchoVariable(b);
			emitir(b.ancho == 1 ? 0xF4 : 0xF5);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Idx && b.indice == 'y') {
			exigirAbs(b);
			emitir(0xF6);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && (b.clase == Clase::Mem || b.clase == Clase::Imm)) {
			if(b.clase == Clase::Imm) {
				emitir(0xE8);
				emitirImm8(b.valor);
				return true;
			}
			anchoVariable(b);
			emitir(b.ancho == 1 ? 0xE4 : 0xE5);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "x" && b.clase == Clase::Imm) {
			emitir(0xCD);
			emitirImm8(b.valor);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "x" && b.clase == Clase::Idx && b.indice == 'y') {
			exigirDp(b);
			emitir(0xF9);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "x" && b.clase == Clase::Mem) {
			anchoVariable(b);
			emitir(b.ancho == 1 ? 0xF8 : 0xE9);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "y" && b.clase == Clase::Imm) {
			emitir(0x8D);
			emitirImm8(b.valor);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "y" && b.clase == Clase::Idx && b.indice == 'x') {
			exigirDp(b);
			emitir(0xFB);
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "y" && b.clase == Clase::Mem) {
			anchoVariable(b);
			emitir(b.ancho == 1 ? 0xEB : 0xEC);
			emitirDireccion(b);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Imm) {
			exigirDp(a);
			emitir(0x8F);
			emitirImm8(b.valor);
			emitirDireccion(a);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && (b.clase == Clase::Mem || b.clase == Clase::Imm)) {
			exigirDp(a);
			exigirDp(b);
			emitir(0xFA);
			emitirDireccion(b);
			emitirDireccion(a);
			return true;
		}
		return false;
	}

	bool codificarAlu(int off, Operando a, Operando b) {
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndX) {
			emitir((uint8_t)(off + 0x06));
			return true;
		}
		if(a.clase == Clase::IndX && b.clase == Clase::IndY) {
			emitir((uint8_t)(off + 0x19));
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Imm) {
			emitir((uint8_t)(off + 0x08));
			emitirImm8(b.valor);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndXP) {
			exigirDp(b);
			emitir((uint8_t)(off + 0x07));
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::IndPY) {
			exigirDp(b);
			emitir((uint8_t)(off + 0x17));
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Idx && b.indice == 'x') {
			anchoVariable(b);
			emitir((uint8_t)(off + (b.ancho == 1 ? 0x14 : 0x15)));
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Idx && b.indice == 'y') {
			exigirAbs(b);
			emitir((uint8_t)(off + 0x16));
			emitirDireccion(b);
			return true;
		}
		if(a.clase == Clase::Reg && a.reg == "a" && b.clase == Clase::Mem) {
			anchoVariable(b);
			emitir((uint8_t)(off + (b.ancho == 1 ? 0x04 : 0x05)));
			emitirDireccion(b);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && b.clase == Clase::Imm) {
			exigirDp(a);
			emitir((uint8_t)(off + 0x18));
			emitirImm8(b.valor);
			emitirDireccion(a);
			return true;
		}
		if((a.clase == Clase::Mem || a.clase == Clase::Imm) && (b.clase == Clase::Mem || b.clase == Clase::Imm)) {
			exigirDp(a);
			exigirDp(b);
			emitir((uint8_t)(off + 0x09));
			emitirDireccion(b);
			emitirDireccion(a);
			return true;
		}
		return false;
	}
};

Valor Ensamblador::parseOr(const std::string &s, size_t &i) {
	Valor izq = parseXor(s, i);
	for(;;) {
		saltar(s, i);
		if(i >= s.size() || s[i] != '|') {
			return izq;
		}
		i++;
		Valor der = parseXor(s, i);
		izq = combinar(izq, der, izq.n | der.n);
	}
}

Valor Ensamblador::parseXor(const std::string &s, size_t &i) {
	Valor izq = parseAnd(s, i);
	for(;;) {
		saltar(s, i);
		if(i >= s.size() || s[i] != '^') {
			return izq;
		}
		i++;
		Valor der = parseAnd(s, i);
		izq = combinar(izq, der, izq.n ^ der.n);
	}
}

Valor Ensamblador::parseAnd(const std::string &s, size_t &i) {
	Valor izq = parseShift(s, i);
	for(;;) {
		saltar(s, i);
		if(i >= s.size() || s[i] != '&') {
			return izq;
		}
		i++;
		Valor der = parseShift(s, i);
		izq = combinar(izq, der, izq.n & der.n);
	}
}

Valor Ensamblador::parseShift(const std::string &s, size_t &i) {
	Valor izq = parseSuma(s, i);
	for(;;) {
		saltar(s, i);
		if(i + 1 >= s.size() || !((s[i] == '<' && s[i + 1] == '<') || (s[i] == '>' && s[i + 1] == '>'))) {
			return izq;
		}
		bool izquierda = s[i] == '<';
		i += 2;
		Valor der = parseSuma(s, i);
		int64_t resultado = 0;
		if(izq.resuelto && der.resuelto) {
			if(der.n < 0 || der.n >= 64) {
				fallar("corrimiento fuera de rango");
			}
			if(izquierda) {
				resultado = (int64_t)((uint64_t)izq.n << der.n);
			} else {
				resultado = (int64_t)((uint64_t)izq.n >> der.n);
			}
		}
		izq = combinar(izq, der, resultado);
	}
}

Valor Ensamblador::parseSuma(const std::string &s, size_t &i) {
	Valor izq = parseMul(s, i);
	for(;;) {
		saltar(s, i);
		if(i >= s.size() || (s[i] != '+' && s[i] != '-')) {
			return izq;
		}
		char op = s[i++];
		Valor der = parseMul(s, i);
		int64_t resultado = (op == '+') ? izq.n + der.n : izq.n - der.n;
		izq = combinar(izq, der, resultado);
	}
}

Valor Ensamblador::parseMul(const std::string &s, size_t &i) {
	Valor izq = parseUnario(s, i);
	for(;;) {
		saltar(s, i);
		if(i >= s.size() || (s[i] != '*' && s[i] != '/' && s[i] != '%')) {
			return izq;
		}
		char op = s[i++];
		Valor der = parseUnario(s, i);
		int64_t resultado = 0;
		if(izq.resuelto && der.resuelto) {
			if((op == '/' || op == '%') && der.n == 0) {
				fallar("division por cero");
			}
			if(op == '*') {
				resultado = izq.n * der.n;
			} else if(op == '/') {
				resultado = izq.n / der.n;
			} else {
				resultado = izq.n % der.n;
			}
		}
		izq = combinar(izq, der, resultado);
	}
}

Valor Ensamblador::parseUnario(const std::string &s, size_t &i) {
	saltar(s, i);
	if(i < s.size() && (s[i] == '+' || s[i] == '-' || s[i] == '~')) {
		char op = s[i++];
		Valor v = parseUnario(s, i);
		if(v.resuelto) {
			if(op == '-') {
				v.n = -v.n;
			} else if(op == '~') {
				v.n = ~v.n;
			}
		}
		return v;
	}
	return parsePrim(s, i);
}

Valor Ensamblador::parsePrim(const std::string &s, size_t &i) {
	saltar(s, i);
	if(i >= s.size()) {
		fallar("expresion vacia");
	}
	if(s[i] == '(') {
		i++;
		Valor v = parseOr(s, i);
		saltar(s, i);
		if(i >= s.size() || s[i] != ')') {
			fallar("parentesis sin cerrar");
		}
		i++;
		return v;
	}
	if(s[i] == '$') {
		i++;
		Valor v;
		v.n = leerDigitos(s, i, 16, "hexadecimal vacio");
		return v;
	}
	if(s[i] == '%') {
		i++;
		Valor v;
		v.n = leerDigitos(s, i, 2, "binario vacio");
		return v;
	}
	if(s[i] == '\'') {
		i++;
		if(i >= s.size()) {
			fallar("caracter sin cerrar");
		}
		char c = s[i++];
		if(c == '\\') {
			if(i >= s.size()) {
				fallar("caracter sin cerrar");
			}
			c = s[i++];
		}
		if(i >= s.size() || s[i] != '\'') {
			fallar("caracter sin cerrar");
		}
		i++;
		Valor v;
		v.n = (unsigned char)c;
		return v;
	}
	if(std::isdigit((unsigned char)s[i])) {
		Valor v;
		v.n = leerDigitos(s, i, 10, "numero vacio");
		return v;
	}
	if(esIdentInicio(s[i])) {
		size_t k = i + 1;
		while(k < s.size() && esIdentCont(s[k])) {
			k++;
		}
		std::string nombre = s.substr(i, k - i);
		i = k;
		Valor v;
		v.usaEtiqueta = true;
		auto it = etiquetas.find(nombre);
		if(it == etiquetas.end()) {
			if(pasada2) {
				fallar("etiqueta no definida: " + nombre);
			}
			v.resuelto = false;
			return v;
		}
		v.n = it->second;
		return v;
	}
	fallar("expresion invalida");
}

} // namespace

// entrada
int EnsamblarProgramaSPC700(std::string &Codigo) {
	Ensamblador ens;
	std::vector<Bloque> bloques = ens.ensamblar(Codigo);
	int retorno = 0;
	for(const Bloque &bloque : bloques) {
		if(bloque.datos.empty()) {
			continue;
		}
		if(bloque.datos.size() > 65535) {
			throw std::runtime_error("spc700: el bloque supera 65535 bytes");
		}
		uint8_t cabecera[4] = {
			(uint8_t)(bloque.datos.size() & 0xFF),
			(uint8_t)((bloque.datos.size() >> 8) & 0xFF),
			(uint8_t)(bloque.aram & 0xFF),
			(uint8_t)((bloque.aram >> 8) & 0xFF),
		};
		cc.EscribirBytes(cabecera, 4);
		cc.EscribirBytes(bloque.datos.data(), (uint32_t)bloque.datos.size());
		retorno = (int)bloque.aram + (int)bloque.datos.size();
	}
	uint8_t finalPrograma[2] = {
		0,
		0};
	cc.EscribirBytes(finalPrograma, 2);
	return retorno;
}