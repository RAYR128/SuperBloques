if(NOT DEFINED SRC OR NOT DEFINED DEST)
	message(FATAL_ERROR "embedir_asm.cmake necesita -DSRC y -DDEST")
endif()

file(READ "${SRC}" HEX_DATA HEX)
string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" "0x\\1," HEX_DATA "${HEX_DATA}")
file(WRITE "${DEST}"
"#pragma once
// Generado desde engine/datos/driverspc.asm. No editar.
static const unsigned char kDriverSpcFuente[] = {
${HEX_DATA}
};
static const unsigned int kDriverSpcFuenteLen = sizeof(kDriverSpcFuente);
")
