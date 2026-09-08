#pragma once

#include "asm/op.h"
#include "bloques.h"

#define xx(n,s) extern void Emit_##n(NodoBloque* blk);
#include "listabloques.h"