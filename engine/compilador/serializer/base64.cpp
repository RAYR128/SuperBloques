#include "base64.h"
#include <cstring>

static const char kTabla[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string CodificarBase64(const uint8_t *Datos, size_t Tamano) {
	std::string Salida;
	Salida.reserve(((Tamano + 2) / 3) * 4);

	size_t i = 0;
	while(i + 2 < Tamano) {
		uint32_t n = ((uint32_t)Datos[i] << 16) | ((uint32_t)Datos[i + 1] << 8) | Datos[i + 2];
		Salida.push_back(kTabla[(n >> 18) & 63]);
		Salida.push_back(kTabla[(n >> 12) & 63]);
		Salida.push_back(kTabla[(n >> 6) & 63]);
		Salida.push_back(kTabla[n & 63]);
		i += 3;
	}

	if(i < Tamano) {
		uint32_t n = (uint32_t)Datos[i] << 16;
		if(i + 1 < Tamano) {
			n |= (uint32_t)Datos[i + 1] << 8;
		}
		Salida.push_back(kTabla[(n >> 18) & 63]);
		Salida.push_back(kTabla[(n >> 12) & 63]);
		if(i + 1 < Tamano) {
			Salida.push_back(kTabla[(n >> 6) & 63]);
			Salida.push_back('=');
		} else {
			Salida.push_back('=');
			Salida.push_back('=');
		}
	}

	return Salida;
}

static int ValorBase64(unsigned char c) {
	if(c >= 'A' && c <= 'Z') {
		return c - 'A';
	}
	if(c >= 'a' && c <= 'z') {
		return c - 'a' + 26;
	}
	if(c >= '0' && c <= '9') {
		return c - '0' + 52;
	}
	if(c == '+') {
		return 62;
	}
	if(c == '/') {
		return 63;
	}
	return -1;
}

static bool DecodificarBase64Vector(const std::string &Entrada, std::vector<uint8_t> &Salida) {
	std::string Limpia;
	Limpia.reserve(Entrada.size());
	for(unsigned char c : Entrada) {
		if(c == ' ' || c == '\n' || c == '\r' || c == '\t') {
			continue;
		}
		Limpia.push_back((char)c);
	}

	Salida.clear();
	if(Limpia.size() % 4 != 0) {
		return false;
	}

	Salida.reserve((Limpia.size() / 4) * 3);
	for(size_t i = 0; i < Limpia.size(); i += 4) {
		int Pad = 0;
		int v[4];
		for(int k = 0; k < 4; k++) {
			char c = Limpia[i + k];
			if(c == '=') {
				if(i + 4 != Limpia.size() || k < 2) {
					Salida.clear();
					return false;
				}
				v[k] = 0;
				Pad++;
			} else {
				if(Pad != 0) {
					Salida.clear();
					return false;
				}
				v[k] = ValorBase64((unsigned char)c);
				if(v[k] < 0) {
					Salida.clear();
					return false;
				}
			}
		}
		if(Pad > 2) {
			Salida.clear();
			return false;
		}

		uint32_t n = ((uint32_t)v[0] << 18) | ((uint32_t)v[1] << 12) | ((uint32_t)v[2] << 6) | (uint32_t)v[3];
		int Bytes = 3 - Pad;
		for(int b = 0; b < Bytes; b++) {
			Salida.push_back((uint8_t)((n >> (16 - 8 * b)) & 0xFF));
		}
	}
	return true;
}

bool DecodificarBase64(const std::string &Entrada, uint8_t *Salida, size_t TamanoEsperado) {
	std::vector<uint8_t> Tmp;
	if(!DecodificarBase64Vector(Entrada, Tmp) || Tmp.size() != TamanoEsperado) {
		return false;
	}
	if(TamanoEsperado > 0) {
		std::memcpy(Salida, Tmp.data(), TamanoEsperado);
	}
	return true;
}

bool DecodificarBase64(const std::string &Entrada, std::vector<uint8_t> &Salida) {
	return DecodificarBase64Vector(Entrada, Salida);
}
