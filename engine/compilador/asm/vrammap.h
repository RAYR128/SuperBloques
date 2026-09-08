#pragma once

// Mapping de VRAM
// $0000-$3FFF: Graficos basicos compartidos por layer 1/2 (4BPP)
// $4000-$4FFF: Tilemap para Layer 1 (512x512)
// $5000-$57FF: Graficos basicos compartidos por layer 3 (2BPP)
// $5800-$5BFF: Tilemap para Layer 2 (256x256)
// $5C00-$5FFF: Tilemap para Layer 3 (256x256)
// $6000-$7FFF: Graficos basicos compartidos por sprites de hardware (4BPP)

#define TILE_SIZE_4BPP 0x20
#define TILE_SIZE_2BPP 0x10

#define ADD_VRAM_GRAFICOS_ESCENA 0x0000
#define ADD_VRAM_TILEMAP_LAYER1 0x4000
#define ADD_VRAM_GRAFICOS_HUD 0x5000
#define ADD_VRAM_TILEMAP_LAYER2 0x5800
#define ADD_VRAM_TILEMAP_LAYER3 0x5C00
#define ADD_VRAM_GRAFICOS_SPRITES 0x6000