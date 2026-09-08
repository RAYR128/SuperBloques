# Diseño de sistema de proyectos

Especificacion en progreso

## Formato

```json
{
	"Escenas": {
		"Escena1": {
			"GraficosPrincipales": ...,
			"GraficosHud": ...,
			"Tilemap1": ...,
			"Tilemap2": ...,
			"Tilemap3": ...,
			"Bloques": {
				...
			}
		}
	},
	"Objetos": {    
		"NuevoObjeto": {
			"Bloques": {
				"bloque_id_001": {
					"PosicionVisual": [1, 1],
					"Operacion": "inicio",
					"Entradas": { ... },
					"Siguiente": "bloque_id_002",
					"Previo": null,
				}
			}
		}
	}
}
```

### Operacion

Operacion del bloque

### PosicionVisual

Posicion visual en el editor.

### Siguiente/Previo

Conexion de nodos

## Encodificacion de datos

Los datos se van a almacenar directamente en el JSON como blobs de base64.