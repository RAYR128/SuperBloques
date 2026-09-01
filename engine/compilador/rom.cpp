#include "ensamblador.h"
#include "mapper.h"

uint8_t DROM[TAMANO_ROM];

uint32_t ConvertirAddrHwAPc(uint32_t addrHw) {
    uint32_t bank = (addrHw >> 16) & 0x7F;
    if(bank < 0x40 && (addrHw & 0x8000)) {
        return (addrHw & 0x7FFF) | (bank << 15);
    }
    return INVALIDO;
}

uint32_t ConvertirAddrPcAHw(uint32_t addrPc) {
    if(addrPc < TAMANO_ROM) {
        uint32_t bank = (addrPc >> 15) & 0x7F;
        return (addrPc & 0x7FFF) | (bank << 16) | 0x8000;
    }
    return INVALIDO;
}

void GuardarROMArchivo(const char* nombreArchivo) {
    FILE* archivo = fopen(nombreArchivo, "wb");
    if(archivo) {
        fwrite(DROM, sizeof(uint8_t), TAMANO_ROM, archivo);
        fclose(archivo);
    }
}