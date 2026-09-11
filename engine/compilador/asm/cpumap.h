// clang-format off
// Generacion utilizando macros para bloques.
#ifndef xx
#define xx(v, r)
#endif

// Cpu
xx(WRAM_DIRECTPAGE, 0x0000) // DP, acceso rapido

// WRAM_SCRATCH se usa de dos formas que nunca coinciden en el tiempo:
// 1) ConRegALlamarJumpTable (dispatch de escena) guarda un word temporal aqui.
// 2) El evaluador de expresiones usa 16 slots de 16-bit (compile-time) para anidar operaciones.
xx(WRAM_SCRATCH, 0x0000)		 // Variables temporales
xx(WRAM_SCRATCH_SIZE, 0x0040) // Tamaño de scratch (0x00-0x3F), utiliza 16-bit (index * 2)

// Layers
xx(WRAM_V_LAYER1_X, 0x00D0)
xx(WRAM_V_LAYER1_Y, 0x00D2)
xx(WRAM_V_LAYER2_X, 0x00D4)
xx(WRAM_V_LAYER2_Y, 0x00D6)
xx(WRAM_V_LAYER3_X, 0x00D8)
xx(WRAM_V_LAYER3_Y, 0x00DA)
xx(WRAM_V_FILTRO_MOSAICO, 0x00DC) // Filtro mosaico (1 byte)
xx(WRAM_V_BRILLO, 0x00DD)		 // Brillo de pantalla (1 byte).. valores 0-15
xx(WRAM_V_COLDATA, 0x00DE)		 // Datos a subir en COLDATA
xx(WRAM_V_QUEUESIZE, 0x00E0)

// La consola nativamente almacena los controladores como variables de 16-bit en HW_CNTRL
// Esto tiene byetudlraxLRxxxx (bit 15..0). Los bits 0-3 son firma del control (no son botones).
xx(WRAM_CONTROL1_MASK, 0x00EC)
xx(WRAM_CONTROL2_MASK, 0x00EE)
xx(WRAM_CONTROL1, 0x00F0)
xx(WRAM_CONTROL1_AXLR_MANTENIDO, 0x00F0)		// Datos de control 1 en bits
xx(WRAM_CONTROL1_BYETUDLR_MANTENIDO, 0x00F1) // Datos de control 1 en bits
xx(WRAM_CONTROL1_PRESIONADO, 0x00F2)
xx(WRAM_CONTROL1_AXLR_PRESIONADO, 0x00F2)	 // Datos de control 1 en bits, solo para el cuadro actual
xx(WRAM_CONTROL1_BYETUDLR_PRESIONADO, 0x00F3) // Datos de control 1 en bits, solo para el cuadro actual

xx(WRAM_CONTROL2, 0x00F4)
xx(WRAM_CONTROL2_AXLR_MANTENIDO, 0x00F4)		// Datos de control 2 en bits
xx(WRAM_CONTROL2_BYETUDLR_MANTENIDO, 0x00F5) // Datos de control 2 en bits
xx(WRAM_CONTROL2_PRESIONADO, 0x00F6)
xx(WRAM_CONTROL2_AXLR_PRESIONADO, 0x00F6)	 // Datos de control 2 en bits, solo para el cuadro actual
xx(WRAM_CONTROL2_BYETUDLR_PRESIONADO, 0x00F7) // Datos de control 2 en bits, solo para el cuadro actual


xx(WRAM_ESCENA_ACTUAL, 0x00F8)  // Escena actual (1 byte)
xx(WRAM_ESCENA_STATUS, 0x00F9)  // Status de escena
xx(WRAM_POSICION_SALTO, 0x00FA) // Posicion salto objeto
xx(WRAM_TIMER, 0x00FD)		   // Timer global
xx(WRAM_FLAG_EJECUCION, 0x00FF) // Sincronizacion con PPU

// Mapping de objetos en la memoria interna.
// 0x0100-0x18FF
xx(WRAM_OBJETOS, 0x0100)

// Mapping de escenas en la memoria interna.
// 0x1900-0x19FF
xx(WRAM_ESCENA, 0x1900)

// Copia de la paleta de 512 bytes
// 0x1A00-0x1BFF
xx(WRAM_PALETA, 0x1A00)

// Pila de la CPU
// 0x1C00-0x1FFF
xx(WRAM_STACK, 0x1FFF)

// Tamaño de WRAM total
xx(WRAM_SIZE, 0x2000)

// Queue de modificacion de tilemaps de WRAM
// 0x7E2000-0x7EFFFF
xx(WRAM_QUEUE_TILEMAP, 0x7E2000)

// Registros de hardware (Memoria especial).
// La consola siempre mapea estos en los bancos $00-$3F, en $2000-$4FFF.
xx(HW_INIDISP, 0x2100)  // Screen brightness & F-blank control
xx(HW_OBJSEL, 0x2101)   // Object size & object data location
xx(HW_OAMADD, 0x2102)   // Word address for OAM access (2 bytes)
xx(HW_OAMDATA, 0x2104)  // OAM data for write (write twice)
xx(HW_BGMODE, 0x2105)   // Background mode and character size
xx(HW_MOSAIC, 0x2106)   // Mosaic effect enable and size
xx(HW_BG1SC, 0x2107)	   // BG1 tilemap address & size
xx(HW_BG2SC, 0x2108)	   // BG2 tilemap address & size
xx(HW_BG3SC, 0x2109)	   // BG3 tilemap address & size
xx(HW_BG4SC, 0x210A)	   // BG4 tilemap address & size
xx(HW_BG12NBA, 0x210B)  // BG1/BG2 character data address
xx(HW_BG34NBA, 0x210C)  // BG3/BG4 character data address
xx(HW_BG1HOFS, 0x210D)  // BG1 horizontal scroll
xx(HW_BG1VOFS, 0x210E)  // BG1 vertical scroll
xx(HW_BG2HOFS, 0x210F)  // BG2 horizontal scroll
xx(HW_BG2VOFS, 0x2110)  // BG2 vertical scroll
xx(HW_BG3HOFS, 0x2111)  // BG3 horizontal scroll
xx(HW_BG3VOFS, 0x2112)  // BG3 vertical scroll
xx(HW_BG4HOFS, 0x2113)  // BG4 horizontal scroll
xx(HW_BG4VOFS, 0x2114)  // BG4 vertical scroll
xx(HW_VMAINC, 0x2115)   // Video port control (VRAM increment)
xx(HW_VMADD, 0x2116)	   // VRAM address (2 bytes)
xx(HW_VMDATA, 0x2118)   // VRAM data write (2 bytes)
xx(HW_M7SEL, 0x211A)	   // Mode 7 settings
xx(HW_MPYA, 0x211B)	   // Mode 7 matrix A (mirror of M7A)
xx(HW_M7A, 0x211B)	   // Mode 7 matrix A
xx(HW_MPYB, 0x211C)	   // Mode 7 matrix B (mirror of M7B)
xx(HW_M7B, 0x211C)	   // Mode 7 matrix B
xx(HW_M7C, 0x211D)	   // Mode 7 matrix C
xx(HW_M7D, 0x211E)	   // Mode 7 matrix D
xx(HW_M7X, 0x211F)	   // Mode 7 center X
xx(HW_M7Y, 0x2120)	   // Mode 7 center Y
xx(HW_CGADD, 0x2121)	   // CGRAM address (palette)
xx(HW_CGDATA, 0x2122)   // CGRAM data write
xx(HW_W12SEL, 0x2123)   // Window mask settings for BG1/BG2
xx(HW_W34SEL, 0x2124)   // Window mask settings for BG3/BG4
xx(HW_WOBJSEL, 0x2125)  // Window mask settings for OBJ/color
xx(HW_WH0, 0x2126)	   // Window 1 left position
xx(HW_WH1, 0x2127)	   // Window 1 right position
xx(HW_WH2, 0x2128)	   // Window 2 left position
xx(HW_WH3, 0x2129)	   // Window 2 right position
xx(HW_WBGLOG, 0x212A)   // Window mask logic for backgrounds
xx(HW_WOBJLOG, 0x212B)  // Window mask logic for OBJ/color
xx(HW_TM, 0x212C)	   // Main screen layer enable
xx(HW_TS, 0x212D)	   // Sub screen layer enable
xx(HW_TMW, 0x212E)	   // Window mask for main screen
xx(HW_TSW, 0x212F)	   // Window mask for sub screen
xx(HW_CGSWSEL, 0x2130)  // Color addition select
xx(HW_CGADSUB, 0x2131)  // Color math designation
xx(HW_COLDATA, 0x2132)  // Fixed color data
xx(HW_SETINI, 0x2133)   // Screen mode / video select
xx(HW_MPY, 0x2134)	   // Multiplication result (3 bytes)
xx(HW_SLHV, 0x2137)	   // Software latch for H/V counter
xx(HW_ROAMDATA, 0x2138) // OAM data read
xx(HW_RVMDATA, 0x2139)  // VRAM data read (2 bytes)
xx(HW_RCGDATA, 0x213B)  // CGRAM data read
xx(HW_OPHCT, 0x213C)	   // Horizontal counter latch
xx(HW_OPVCT, 0x213D)	   // Vertical counter latch
xx(HW_STAT77, 0x213E)   // PPU1 status flags
xx(HW_STAT78, 0x213F)   // PPU2 status flags
xx(HW_APUIO0, 0x2140)   // APU I/O port 0
xx(HW_APUIO1, 0x2141)   // APU I/O port 1
xx(HW_APUIO2, 0x2142)   // APU I/O port 2
xx(HW_APUIO3, 0x2143)   // APU I/O port 3
xx(HW_WMDATA, 0x2180)   // WRAM data read/write
xx(HW_WMADD, 0x2181)	   // WRAM address (3 bytes)

xx(HW_JOY1, 0x4016) // Joypad 1 access (NES-style)
xx(HW_JOY2, 0x4017) // Joypad 2 access (NES-style)

xx(HW_NMITIMEN, 0x4200) // Interrupt enable flags
xx(HW_WRIO, 0x4201)	   // Programmable I/O port (out)
xx(HW_WRMPYA, 0x4202)   // Multiplicand A
xx(HW_WRMPYB, 0x4203)   // Multiplicand B
xx(HW_WRDIV, 0x4204)	   // Dividend (2 bytes)
xx(HW_WRDIVB, 0x4206)   // Divisor
xx(HW_HTIME, 0x4207)	   // H-count timer setting (2 bytes)
xx(HW_VTIME, 0x4209)	   // V-count timer setting (2 bytes)
xx(HW_MDMAEN, 0x420B)   // DMA channel enable
xx(HW_HDMAEN, 0x420C)   // HDMA channel enable
xx(HW_MEMSEL, 0x420D)   // ROM access speed (FastROM)
xx(HW_RDNMI, 0x4210)	   // NMI flag and 5A22 version
xx(HW_TIMEUP, 0x4211)   // IRQ flag
xx(HW_HVBJOY, 0x4212)   // H/V blank & joypad status
xx(HW_RDIO, 0x4213)	   // Programmable I/O port (in)
xx(HW_RDDIV, 0x4214)	   // Unsigned division result (2 bytes)
xx(HW_RDMPY, 0x4216)	   // Unsigned multiplication / remainder (2 bytes)
xx(HW_CNTRL1, 0x4218)   // Controller port 1 data (2 bytes)
xx(HW_CNTRL2, 0x421A)   // Controller port 2 data (2 bytes)
xx(HW_CNTRL3, 0x421C)   // Controller port 3 data (2 bytes)
xx(HW_CNTRL4, 0x421E)   // Controller port 4 data (2 bytes)

xx(HW_DMAPARAM, 0x4300)	// DMA transfer parameters
xx(HW_DMAREG, 0x4301)	// B-bus register to transfer to/from
xx(HW_DMAADDR, 0x4302)	// A-bus 24-bit address (3 bytes)
xx(HW_DMACNT, 0x4305)	// DMA byte count / HDMA table address (2 bytes)
xx(HW_HDMABANK, 0x4307)	// HDMA indirect table bank
xx(HW_DMAIDX, 0x4308)	// HDMA intermediate address (2 bytes)
xx(HW_HDMALINES, 0x430A) // HDMA line counter


#undef xx
																								   // clang-format on
