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
			"Paleta": "<base64>",
			"Variables": ["Variable 1"],
			"Bloques": {
				"bloque_id_001": {
					"PosicionVisual": [40, 40],
					"Operacion": "evento_init",
					"ParametroEspecial": 0,
					"Entradas": [],
					"Siguiente": "bloque_id_002",
					"Previo": null
				},
				"bloque_id_002": {
					"PosicionVisual": [1, 2],
					"Operacion": "motion_set_posicion_x",
					"ParametroEspecial": 0,
					"Entradas": [],
					"Siguiente": "bloque_id_003",
					"Previo": "bloque_id_001"
				},
				"bloque_id_003": {
					"PosicionVisual": [1, 3],
					"Operacion": "variable",
					"ParametroEspecial": "Variable 1",
					"Entradas": [],
					"Siguiente": null,
					"Previo": "bloque_id_002"
				}
			}
		}
	},
	"Objetos": {
		"NuevoObjeto": {
			"Variables": ["Variable 1"],
			"Bloques": {
				"bloque_id_001": {
					"PosicionVisual": [40, 40],
					"Operacion": "evento_init",
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

`Escenas` y `Objetos` pueden omitirse (se tratan como `{}`). Un blob omitido se carga como ceros. Un JSON malformado no modifica el proyecto en memoria, un error de contenido despues del parse (Operacion desconocida, base64 de tamaño incorrecto, id colgante, variable inexistente o duplicada, demasiadas variables) tampoco, porque se construye un proyecto temporal y solo se publica al terminar.

### Operacion

String que identifica `TipoBloque`.

### Variables

Array de nombres unicos en la escena u objeto. El indice en este array es el offset que usa un bloque `variable` / `variable_store`. Si se omite, se trata como `[]`. Nombres vacios o duplicados se rechazan.

### ParametroEspecial

Entero auxiliar del bloque (por ejemplo el literal de `numero`), o un string con el nombre de una variable del mismo objeto/escena. Un string se resuelve al offset (indice) de ese nombre en `Variables`; si no existe, es un error de contenido. Un entero se usa tal cual (para `variable` / `variable_store` es el offset). Si se omite, vale `0`. Al exportar, `variable` y `variable_store` escriben el nombre si el offset apunta a una entrada de `Variables`.

### PosicionVisual

Par `[x, y]` en pixeles del editor. Lo usan los bloques raiz de una pila (sin `Previo` y que no son destino de `Cuerpo` / `CuerpoSino`). Los sub-bloques de `Entradas` pueden omitirlo.

### Entradas

Array de bloques anidados por valor (no ids). Cada entrada es un objeto con `Operacion`, `ParametroEspecial` y `Entradas` propias. En `control_if`, `control_while` y `control_ifelse`, `Entradas[0]` es la condicion (bloque de valor).

### Siguiente / Previo

Id de otro bloque en el mismo mapa `Bloques`, o `null`. El id solo existe en el JSON; en memoria son punteros entre elementos del vector de pila. Los bloques anidados en `Entradas` no participan de este mapa.

Al exportar, los ids se generan como `bloque_id_001`, `bloque_id_002`, ... en el orden del vector.

### Cuerpo / CuerpoSino

Id de otro bloque en el mismo mapa `Bloques`, o se omiten. Encabezan las subpilas de statement de un bloque C:

| Operacion | `Entradas[0]` | `Cuerpo` | `CuerpoSino` |
|---|---|---|---|
| `control_if` | condicion (valor) | pila si verdad | no aplicable |
| `control_while` | condicion (valor) | pila del cuerpo | no aplicable |
| `control_ifelse` | condicion (valor) | pila si verdad | pila si falso |

Los bloques de esas subpilas viven en `Bloques` y se encadenan con `Siguiente` / `Previo`, igual que el resto de la pila. Un id colgante se rechaza.

## Encodificacion de datos

Los buffers de cada escena se almacenan como blobs base64 de tamaño fijo. Un decode cuyo binario no coincida exactamente se rechaza.

| Campo | Bytes |
|---|---|
| `GraficosPrincipales` | `0x400 * TILE_SIZE_4BPP` = 32768 |
| `GraficosHud` | `0x100 * TILE_SIZE_2BPP` = 4096 |
| `Tilemap1` | `64 * 64 * 2` = 8192 |
| `Tilemap2` | `32 * 32 * 2` = 2048 |
| `Tilemap3` | `32 * 32 * 2` = 2048 |
| `Paleta` | `256 * 2` = 512 |
