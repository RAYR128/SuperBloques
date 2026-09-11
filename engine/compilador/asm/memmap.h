#pragma once

#include "vrammap.h"

// Objetos
#define CANTIDAD_DE_OBJETOS 64 // 64 objetos en la pantalla maximo.
#define TAMANO_OBJETO 96	   // Cada objeto ocupa 96 bytes en la memoria interna
#define TAMANO_ESCENA 256	   // La escena actual tiene 256 bytes (128 variables) disponibles para trabajar en la memoria interna

#define OBJETO_VARIABLES_MAX 43
#define ESCENA_VARIABLES_MAX 128

enum ParametrosObjeto {
	PARAMETRO_OBJ_BHV_SCRIPT_STATUS = 0,  // 1 byte para un estado de este objeto, 0 = no existe, 1-255 = usar como jump table
	PARAMETRO_OBJ_BHV_SCRIPT_POINTER = 1, // 3 bytes para una ubicacion en PC
	PARAMETRO_OBJ_POSICION_X = 4,		  // 2 bytes para la posicion X del objeto
	PARAMETRO_OBJ_POSICION_Y = 6,		  // 2 bytes para la posicion Y del objeto
	PARAMETRO_OBJ_SPRITE = 8,			  // 2 bytes para la frame del objeto
	PARAMETRO_OBJ_VARIABLES = 10		  // 86 bytes para variables del objeto (43 variables 16-bit), dando un total de 96 bytes por objeto.
};

enum DireccionesCPU {
#define xx(v, r) v = r,
#include "cpumap.h"
};

// ParametroEspecial de SENSOR_BOTON_CONTROL_1/2. Los primeros bits de firma (BOTON_FIRMA_0..3) no corresponden a un boton, pero igual deben compilar.
enum BotonControl {
	BOTON_FIRMA_0 = 0,
	BOTON_FIRMA_1 = 1,
	BOTON_FIRMA_2 = 2,
	BOTON_FIRMA_3 = 3,
	BOTON_R = 4,
	BOTON_L = 5,
	BOTON_X = 6,
	BOTON_A = 7,
	BOTON_DERECHA = 8,
	BOTON_IZQUIERDA = 9,
	BOTON_ABAJO = 10,
	BOTON_ARRIBA = 11,
	BOTON_START = 12,
	BOTON_SELECT = 13,
	BOTON_Y = 14,
	BOTON_B = 15
};

// Parametros de DMA (HW_DMAPARAM). Se combinan con OR.
enum ParametrosDMA {
	HW_DMA_1Byte1Addr = 0x00,	// %000
	HW_DMA_2Byte2Addr = 0x01,	// %001
	HW_DMA_2Byte1Addr = 0x02,	// %010
	HW_DMA_4Byte2Addr = 0x03,	// %011
	HW_DMA_4Byte4Addr = 0x04,	// %100
	HW_DMA_ABusInc = 0x00,		// %00000
	HW_DMA_ABusDec = 0x10,		// %10000
	HW_DMA_ABusFix = 0x08,		// %01000
	HW_DMA_HDMAIndirect = 0x40, // %01000000
	HW_DMA_AtoB = 0x00,			// %00000000
	HW_DMA_BtoA = 0x80			// %10000000
};

// Parametros de tamaño de BG, para HW_BGXSC
enum ParametroBGSize {
	HW_BGSC_Size_32x32 = 0,
	HW_BGSC_Size_64x32 = 1,
	HW_BGSC_Size_32x64 = 2,
	HW_BGSC_Size_64x64 = 3
};

// Para HW_TM/TS/TMW/TSW
enum ParametroLayerThrough {
	HW_Through_BG1 = 1,
	HW_Through_BG2 = 2,
	HW_Through_BG3 = 4,
	HW_Through_BG4 = 8,
	HW_Through_OBJ = 16
};

// Para HW_CGWSEL
enum {
	HW_CGWSEL_DIRECTCOLOR = 0x01,	// 8bpp modes only
	HW_CGWSEL_USE_SUBSCREEN = 0x02, // 0 = fixed color (COLDATA), 1 = TS layers
	HW_CGWSEL_MATH_NEVER = 0x00,
	HW_CGWSEL_MATH_OUTSIDEWIN = 0x10,
	HW_CGWSEL_MATH_INSIDEWIN = 0x20,
	HW_CGWSEL_MATH_ALWAYS = 0x30,
	HW_CGWSEL_CLIP_NEVER = 0x00,
	HW_CGWSEL_CLIP_OUTSIDEWIN = 0x40,
	HW_CGWSEL_CLIP_INSIDEWIN = 0x80,
	HW_CGWSEL_CLIP_ALWAYS = 0xC0
};

// Para HW_CGADSUB
enum {
	HW_CGADSUB_BG1 = 0x01,
	HW_CGADSUB_BG2 = 0x02,
	HW_CGADSUB_BG3 = 0x04,
	HW_CGADSUB_BG4 = 0x08,
	HW_CGADSUB_OBJ = 0x10, // sprites palettes 4-7 only
	HW_CGADSUB_BACKDROP = 0x20,
	HW_CGADSUB_HALF = 0x40,
	HW_CGADSUB_SUBTRACT = 0x80 // 0 = add
};