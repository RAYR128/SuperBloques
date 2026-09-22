#pragma once

#include <string>

// Carga el JSON, borra la ROM y la vuelve a ensamblar en DROM.
// Se puede llamar mas de una vez en el mismo proceso.
void CompilarProyectoEnMemoria(const std::string &json);
