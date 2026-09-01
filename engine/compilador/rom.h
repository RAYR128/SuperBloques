#pragma once

#include <cstdint>

#define TAMANO_ROM 2*1024*1024 // 2MB
#define INVALIDO 0xFFFFFFFF

extern uint8_t DROM[TAMANO_ROM];
extern uint32_t ConvertirAddrHwAPc(uint32_t addrHw);
extern uint32_t ConvertirAddrPcAHw(uint32_t addrPc);
extern void InicializarROM();
extern void GuardarROMArchivo(const char* nombreArchivo);