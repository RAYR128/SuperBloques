#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

std::string CodificarBase64(const uint8_t *Datos, size_t Tamano);
bool DecodificarBase64(const std::string &Entrada, uint8_t *Salida, size_t TamanoEsperado);
