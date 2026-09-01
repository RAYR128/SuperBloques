cmake_minimum_required(VERSION 3.16)
project(SuperBloques LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(set_output_dirs target dir)
	set_target_properties(${target} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${dir}"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${dir}"
	)
endfunction()

add_executable(superbloques_cli
	engine/compilador/main.cpp
	engine/compilador/rom.cpp
	engine/compilador/instancia.cpp
	engine/compilador/bloques.cpp
	engine/compilador/asm/op.cpp
	engine/compilador/asm/ensamblador.cpp
	engine/compilador/codegen/bloques_motion.cpp
)
target_include_directories(superbloques_cli PRIVATE
	${CMAKE_SOURCE_DIR}/engine/compilador
)

set_output_dirs(superbloques_cli "..")