#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

std::string CodificarBase64(const uint8_t *Datos, size_t Tamano);
bool DecodificarBase64(const std::string &Entrada, uint8_t *Salida, size_t TamanoEsperado);
bool DecodificarBase64(const std::string &Entrada, std::vector<uint8_t> &Salida);
