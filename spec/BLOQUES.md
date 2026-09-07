# Diseño de sistema de bloques

Especificacion en progreso

## Idea

´´´json
{
    "Nombre": "NuevoObjeto",
    "Tipo": "Objeto",
    "Bloques": {
        "bloque_id_001": {
            "PosicionVisual": [1, 1]
            "Operacion": "inicio",
            "Entradas": { ... }
            "Siguiente": "bloque_id_002",
            "Previo": null,
        }
    }
}
´´´

### Operacion

Operacion del bloque

### PosicionVisual

Posicion visual en el editor.

### Siguiente/Previo

Conexion de nodos