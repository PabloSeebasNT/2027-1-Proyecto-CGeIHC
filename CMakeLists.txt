cmake_minimum_required(VERSION 3.16)
project(ProyectoFinal CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
pkg_check_modules(GLEW REQUIRED IMPORTED_TARGET glew)

# assimp: carga de modelos OBJ (Model.h/Model.cpp)
pkg_check_modules(ASSIMP IMPORTED_TARGET assimp)
if(NOT ASSIMP_FOUND)
	find_path(ASSIMP_INCLUDE_DIR NAMES assimp/scene.h
		PATHS /opt/homebrew/include /usr/local/include)
	find_library(ASSIMP_LIBRARY NAMES assimp
		PATHS /opt/homebrew/lib /usr/local/lib)
	if(NOT ASSIMP_INCLUDE_DIR OR NOT ASSIMP_LIBRARY)
		message(FATAL_ERROR "No se encontro assimp. Instala con: brew install assimp")
	endif()
endif()

# Los fuentes del curso incluyen <glew.h>, <glfw3.h> y <glm.hpp> sin carpeta: se busca la carpeta de cada uno.
# Funciona con Homebrew (macOS) y con los paquetes -dev de Linux
set(RUTAS_INCLUDE /opt/homebrew/include /usr/local/include /usr/include ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
find_path(GLFW_H_DIR NAMES glfw3.h PATHS ${RUTAS_INCLUDE} PATH_SUFFIXES GLFW)
find_path(GLEW_H_DIR NAMES glew.h PATHS ${RUTAS_INCLUDE} PATH_SUFFIXES GL)
find_path(GLM_H_DIR NAMES glm.hpp PATHS ${RUTAS_INCLUDE} PATH_SUFFIXES glm)
if(NOT GLFW_H_DIR OR NOT GLEW_H_DIR OR NOT GLM_H_DIR)
	message(FATAL_ERROR "No se encontraron glfw3.h, glew.h o glm.hpp. En macOS: brew install glfw glew glm")
endif()

add_executable(ProyectoFinal
    CircuitoEspacial.cpp
    CarreraEspacial.cpp
    EstacionEspacial.cpp
	ProyectoFinal.cpp
	Geometria.cpp
	Avatar.cpp
	Pyros.cpp
	PlanetaOoo.cpp
	Finn.cpp
	OooPaisaje.cpp
	OooCasaArbol.cpp
	OooModelosNature.cpp
	OooJardin.cpp
    OooLuces.cpp
    PersonajesOoo.cpp
    AmigosCodigoOoo.cpp
    GrumosaOoo.cpp
    MentaOoo.cpp
    OooEvolucion.cpp
    ArcoirisOoo.cpp
	NaveVerde.cpp
	Camera.cpp
	Window.cpp
	Shader_light.cpp
	Mesh_tn.cpp
	Model.cpp
	Texture.cpp
	Skybox.cpp
	Light.cpp
	DirectionalLight.cpp
	PointLight.cpp
	SpotLight.cpp
	Material.cpp
)

target_include_directories(ProyectoFinal PRIVATE ${GLFW_H_DIR} ${GLEW_H_DIR} ${GLM_H_DIR})

if(ASSIMP_FOUND)
	target_include_directories(ProyectoFinal PRIVATE ${ASSIMP_INCLUDE_DIRS})
	target_link_libraries(ProyectoFinal PRIVATE PkgConfig::ASSIMP)
else()
	target_include_directories(ProyectoFinal PRIVATE ${ASSIMP_INCLUDE_DIR})
	target_link_libraries(ProyectoFinal PRIVATE ${ASSIMP_LIBRARY})
endif()

target_link_libraries(ProyectoFinal PRIVATE
	PkgConfig::GLFW
	PkgConfig::GLEW
	OpenGL::GL
)

# Copiar shaders, modelos y texturas junto al ejecutable para que funcionen
# las rutas relativas ("shaders/...", "Models/...", "Textures/...").
add_custom_command(TARGET ProyectoFinal POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/shaders
		$<TARGET_FILE_DIR:ProyectoFinal>/shaders
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/Models
		$<TARGET_FILE_DIR:ProyectoFinal>/Models
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/Textures
		$<TARGET_FILE_DIR:ProyectoFinal>/Textures
)


