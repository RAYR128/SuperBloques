# Diseño de sistema de proyectos

Especificacion en progreso

## Formato

Un proyecto es un objeto JSON con dos mapas: `Escenas` y `Objetos`. Las keys son los nombres. Los blobs binarios de cada escena van como strings base64 RFC 4648 (alfabeto `A-Za-z0-9+/`, padding `=`).

```json
{
	"Escenas": {
		"Escena1": {
			"GraficosPrincipales": "<base64>",
			"GraficosHud": "<base64>",
			"Tilemap1": "<base64>",
			"Tilemap2": "<base64>",
			"Tilemap3": "<base64>",
			"Bloques": {
				"bloque_id_001": {
					"PosicionVisual": [1, 1],
					"Operacion": "evento",
					"ParametroEspecial": 0,
					"Entradas": [
						{
							"Operacion": "numero",
							"ParametroEspecial": 16,
							"Entradas": []
						}
					],
					"Siguiente": "bloque_id_002",
					"Previo": null
				},
				"bloque_id_002": {
					"PosicionVisual": [1, 2],
					"Operacion": "motion_set_posicion_x",
					"ParametroEspecial": 0,
					"Entradas": [],
					"Siguiente": null,
					"Previo": "bloque_id_001"
				}
			}
		}
	},
	"Objetos": {
		"NuevoObjeto": {
			"Bloques": {
				"bloque_id_001": {
					"PosicionVisual": [0, 0],
					"Operacion": "evento",
					"ParametroEspecial": 0,
					"Entradas": [],
					"Siguiente": null,
					"Previo": null
				}
			}
		}
	}
}
```

`Escenas` y `Objetos` pueden omitirse (se tratan como `{}`). Un blob omitido se carga como ceros. Un JSON malformado no modifica el proyecto en memoria, un error de contenido despues del parse (Operacion desconocida, base64 de tamaño incorrecto, id colgante) tampoco, porque se construye un proyecto temporal y solo se publica al terminar.

### Operacion

String que identifica `TipoBloque`.

### ParametroEspecial

Entero auxiliar del bloque (por ejemplo el literal de `numero`). Si se omite, vale `0`.

### PosicionVisual

Par `[x, y]` en el editor. Solo lo usan los bloques de pila (el bloque mas superior de un script). Los sub-bloques de `Entradas` pueden omitirlo.

### Entradas

Array de bloques anidados por valor (no ids). Cada entrada es un objeto con `Operacion`, `ParametroEspecial` y `Entradas` propias.

### Siguiente / Previo

Id de otro bloque en el mismo mapa `Bloques`, o `null`. El id solo existe en el JSON; en memoria son punteros entre elementos del vector de pila. Los bloques anidados en `Entradas` no participan de este mapa.

Al exportar, los ids se generan como `bloque_id_001`, `bloque_id_002`, ... en el orden del vector.

## Encodificacion de datos

Los buffers de cada escena se almacenan como blobs base64 de tamaño fijo. Un decode cuyo binario no coincida exactamente se rechaza.

| Campo | Bytes |
|---|---|
| `GraficosPrincipales` | `0x400 * TILE_SIZE_4BPP` = 32768 |
| `GraficosHud` | `0x100 * TILE_SIZE_2BPP` = 4096 |
| `Tilemap1` | `64 * 64 * 2` = 8192 |
| `Tilemap2` | `32 * 32 * 2` = 2048 |
| `Tilemap3` | `32 * 32 * 2` = 2048 |
