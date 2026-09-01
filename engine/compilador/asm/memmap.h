#pragma once

// Mapping de objetos en la memoria interna.
// 0x100-0x1900
#define WRAM_DIRECTPAGE 0x0000
#define WRAM_OBJETOS 0x0100

#define CANTIDAD_DE_OBJETOS 64 // 64 objetos en la pantalla maximo.
#define TAMANO_OBJETO 96 // Cada objeto ocupa 96 bytes en la memoria interna
#define PARAMETRO_OBJ_BHV_SCRIPT_STATUS 0 // 1 byte para un estado de este objeto, 0 = no existe, 1-255 = usar como jump table
#define PARAMETRO_OBJ_BHV_SCRIPT_POINTER 1 // 3 bytes para una ubicacion en PC
#define PARAMETRO_OBJ_POSICION_X 4 // 2 bytes para la posicion X del objeto
#define PARAMETRO_OBJ_POSICION_Y 6 // 2 bytes para la posicion Y del objeto
#define PARAMETRO_OBJ_SPRITE 8 // 2 bytes para la frame del objeto
#define PARAMETRO_OBJ_VARIABLES 10 // 86 bytes para variables del objeto (43 variables 16-bit), dando un total de 96 bytes por objeto.

#define WRAM_STACK 0x1FFF // Pila de la CPU

// Registros de hardware (Memoria especial).
// La consola siempre mapea estos en los bancos $00-$3F, en $2000-$4FFF.
#define HW_INIDISP 0x2100 // Screen brightness & F-blank control
#define HW_OBJSEL 0x2101 // Object size & object data location
#define HW_OAMADD 0x2102 // Word address for OAM access (2 bytes)
#define HW_OAMDATA 0x2104 // OAM data for write (write twice)
#define HW_BGMODE 0x2105 // Background mode and character size
#define HW_MOSAIC 0x2106 // Mosaic effect enable and size
#define HW_BG1SC 0x2107 // BG1 tilemap address & size
#define HW_BG2SC 0x2108 // BG2 tilemap address & size
#define HW_BG3SC 0x2109 // BG3 tilemap address & size
#define HW_BG4SC 0x210A // BG4 tilemap address & size
#define HW_BG12NBA 0x210B // BG1/BG2 character data address
#define HW_BG34NBA 0x210C // BG3/BG4 character data address
#define HW_BG1HOFS 0x210D // BG1 horizontal scroll
#define HW_BG1VOFS 0x210E // BG1 vertical scroll
#define HW_BG2HOFS 0x210F // BG2 horizontal scroll
#define HW_BG2VOFS 0x2110 // BG2 vertical scroll
#define HW_BG3HOFS 0x2111 // BG3 horizontal scroll
#define HW_BG3VOFS 0x2112 // BG3 vertical scroll
#define HW_BG4HOFS 0x2113 // BG4 horizontal scroll
#define HW_BG4VOFS 0x2114 // BG4 vertical scroll
#define HW_VMAINC 0x2115 // Video port control (VRAM increment)
#define HW_VMADD 0x2116 // VRAM address (2 bytes)
#define HW_VMDATA 0x2118 // VRAM data write (2 bytes)
#define HW_M7SEL 0x211A // Mode 7 settings
#define HW_MPYA 0x211B // Mode 7 matrix A (mirror of M7A)
#define HW_M7A 0x211B // Mode 7 matrix A
#define HW_MPYB 0x211C // Mode 7 matrix B (mirror of M7B)
#define HW_M7B 0x211C // Mode 7 matrix B
#define HW_M7C 0x211D // Mode 7 matrix C
#define HW_M7D 0x211E // Mode 7 matrix D
#define HW_M7X 0x211F // Mode 7 center X
#define HW_M7Y 0x2120 // Mode 7 center Y
#define HW_CGADD 0x2121 // CGRAM address (palette)
#define HW_CGDATA 0x2122 // CGRAM data write
#define HW_W12SEL 0x2123 // Window mask settings for BG1/BG2
#define HW_W34SEL 0x2124 // Window mask settings for BG3/BG4
#define HW_WOBJSEL 0x2125 // Window mask settings for OBJ/color
#define HW_WH0 0x2126 // Window 1 left position
#define HW_WH1 0x2127 // Window 1 right position
#define HW_WH2 0x2128 // Window 2 left position
#define HW_WH3 0x2129 // Window 2 right position
#define HW_WBGLOG 0x212A // Window mask logic for backgrounds
#define HW_WOBJLOG 0x212B // Window mask logic for OBJ/color
#define HW_TM 0x212C // Main screen layer enable
#define HW_TS 0x212D // Sub screen layer enable
#define HW_TMW 0x212E // Window mask for main screen
#define HW_TSW 0x212F // Window mask for sub screen
#define HW_CGSWSEL 0x2130 // Color addition select
#define HW_CGADSUB 0x2131 // Color math designation
#define HW_COLDATA 0x2132 // Fixed color data
#define HW_SETINI 0x2133 // Screen mode / video select
#define HW_MPY 0x2134 // Multiplication result (3 bytes)
#define HW_SLHV 0x2137 // Software latch for H/V counter
#define HW_ROAMDATA 0x2138 // OAM data read
#define HW_RVMDATA 0x2139 // VRAM data read (2 bytes)
#define HW_RCGDATA 0x213B // CGRAM data read
#define HW_OPHCT 0x213C // Horizontal counter latch
#define HW_OPVCT 0x213D // Vertical counter latch
#define HW_STAT77 0x213E // PPU1 status flags
#define HW_STAT78 0x213F // PPU2 status flags
#define HW_APUIO0 0x2140 // APU I/O port 0
#define HW_APUIO1 0x2141 // APU I/O port 1
#define HW_APUIO2 0x2142 // APU I/O port 2
#define HW_APUIO3 0x2143 // APU I/O port 3
#define HW_WMDATA 0x2180 // WRAM data read/write
#define HW_WMADD 0x2181 // WRAM address (3 bytes)

#define HW_JOY1 0x4016 // Joypad 1 access (NES-style)
#define HW_JOY2 0x4017 // Joypad 2 access (NES-style)

#define HW_NMITIMEN 0x4200 // Interrupt enable flags
#define HW_WRIO 0x4201 // Programmable I/O port (out)
#define HW_WRMPYA 0x4202 // Multiplicand A
#define HW_WRMPYB 0x4203 // Multiplicand B
#define HW_WRDIV 0x4204 // Dividend (2 bytes)
#define HW_WRDIVB 0x4206 // Divisor
#define HW_HTIME 0x4207 // H-count timer setting (2 bytes)
#define HW_VTIME 0x4209 // V-count timer setting (2 bytes)
#define HW_MDMAEN 0x420B // DMA channel enable
#define HW_HDMAEN 0x420C // HDMA channel enable
#define HW_MEMSEL 0x420D // ROM access speed (FastROM)
#define HW_RDNMI 0x4210 // NMI flag and 5A22 version
#define HW_TIMEUP 0x4211 // IRQ flag
#define HW_HVBJOY 0x4212 // H/V blank & joypad status
#define HW_RDIO 0x4213 // Programmable I/O port (in)
#define HW_RDDIV 0x4214 // Unsigned division result (2 bytes)
#define HW_RDMPY 0x4216 // Unsigned multiplication / remainder (2 bytes)
#define HW_CNTRL1 0x4218 // Controller port 1 data (2 bytes)
#define HW_CNTRL2 0x421A // Controller port 2 data (2 bytes)
#define HW_CNTRL3 0x421C // Controller port 3 data (2 bytes)
#define HW_CNTRL4 0x421E // Controller port 4 data (2 bytes)

#define HW_DMAPARAM 0x4300 // DMA transfer parameters
#define HW_DMAREG 0x4301 // B-bus register to transfer to/from
#define HW_DMAADDR 0x4302 // A-bus 24-bit address (3 bytes)
#define HW_DMACNT 0x4305 // DMA byte count / HDMA table address (2 bytes)
#define HW_HDMABANK 0x4307 // HDMA indirect table bank
#define HW_DMAIDX 0x4308 // HDMA intermediate address (2 bytes)
#define HW_HDMALINES 0x430A // HDMA line counter