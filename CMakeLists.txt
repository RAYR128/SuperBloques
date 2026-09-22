cmake_minimum_required(VERSION 3.16)
project(SuperBloques LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SUPERBLOQUES_WASM "Compilar el compilador a WebAssembly" OFF)

function(set_output_dirs target dir)
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${dir}"
	)
endfunction()

set(COMPILADOR_SOURCES
	engine/compilador/compilar.cpp
	engine/compilador/rom.cpp
	engine/compilador/datos.cpp
	engine/compilador/ensamblador.cpp
	engine/compilador/instancia.cpp
	engine/compilador/bloques.cpp
	engine/compilador/asm/op.cpp
	engine/compilador/serializer/cargar.cpp
	engine/compilador/serializer/guardar.cpp
	engine/compilador/serializer/base64.cpp
	engine/compilador/codegen/codegen.cpp
	engine/compilador/codegen/bloques_motion.cpp
	engine/compilador/codegen/bloques_evento.cpp
	engine/compilador/codegen/bloques_variable.cpp
	engine/compilador/codegen/bloques_operacion.cpp
	engine/compilador/codegen/bloques_control.cpp
	engine/compilador/codegen/bloques_animacion.cpp
	engine/compilador/codegen/bloques_capa.cpp
	engine/compilador/codegen/bloques_sonido.cpp
	engine/compilador/codegen/bloques_sensor.cpp
)

set(COMPILADOR_INCLUDES
	${CMAKE_SOURCE_DIR}/engine/compilador
	${CMAKE_SOURCE_DIR}/engine/lib
)

if(SUPERBLOQUES_WASM)
	add_executable(superbloques_wasm
		${COMPILADOR_SOURCES}
		engine/compilador/wasm_api.cpp
	)
	target_include_directories(superbloques_wasm PRIVATE ${COMPILADOR_INCLUDES})
	target_compile_options(superbloques_wasm PRIVATE -fwasm-exceptions -O2)
	target_link_options(superbloques_wasm PRIVATE
		-fwasm-exceptions
		-O2
		"SHELL:-s MODULARIZE=1"
		"SHELL:-s EXPORT_ES6=1"
		"SHELL:-s ENVIRONMENT=web,node"
		"SHELL:-s FILESYSTEM=0"
		"SHELL:-s ALLOW_MEMORY_GROWTH=1"
		"SHELL:-s INITIAL_MEMORY=33554432"
		"SHELL:-s STACK_SIZE=1048576"
		"SHELL:-s MAXIMUM_MEMORY=268435456"
		"SHELL:-s EXPORTED_FUNCTIONS=[_sb_compilar,_sb_rom,_sb_rom_tamano,_sb_ultimo_error,_malloc,_free]"
		"SHELL:-s EXPORTED_RUNTIME_METHODS=[UTF8ToString,HEAPU8]"
	)
	set_target_properties(superbloques_wasm PROPERTIES OUTPUT_NAME superbloques)
	set_output_dirs(superbloques_wasm "${CMAKE_SOURCE_DIR}/frontend/editor/public/compilador")
else()
	add_executable(superbloques_cli
		engine/compilador/main.cpp
		${COMPILADOR_SOURCES}
	)
	target_include_directories(superbloques_cli PRIVATE ${COMPILADOR_INCLUDES})
	set_output_dirs(superbloques_cli "..")
endif()