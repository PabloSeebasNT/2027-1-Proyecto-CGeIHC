# Etapa 1: Ooo registrado

Se agregó una zona nueva a petición de Uri. No se ocuparon ni renombraron los seis planetas existentes.

- Nombre: Ooo (Hora de Aventura).
- Responsable: Uri.
- Índice: 6 (séptimo planeta).
- Centro en el espacio: (-330, 0, 430), radio 42.
- Centro de superficie: (22000, 0, 0), radio caminable 110.
- Aparición del avatar: (22000, 0, 30).
- Piso provisional: 400 x 400, verde, texturizado.
- Skybox: espacio compartido; la ambientación propia se incorporará después.

## Probar

Recargar la solución si Visual Studio lo solicita. Seleccionar Release / x64, compilar y ejecutar con Ctrl+F5.

Desde el inicio en Pyros, presionar y soltar P seis veces para llegar a Ooo. En la consola aparece `Avatar en: Ooo (Hora de Aventura) (Uri)`. Una pulsación más regresa a Pyros. Usar cámara 1, W/S para avanzar/retroceder y A/D o ratón para girar. H adelanta la hora y L activa la linterna.

## Archivos

- PlanetaOoo.h: posiciones, dimensiones, color, punto de aparición y funciones públicas.
- PlanetaOoo.cpp: carga del piso y texturas; dibujo de superficie y esfera espacial.
- ProyectoFinal.cpp: registro, carga y llamadas de dibujo; punto de aparición de Ooo.
- ProyectoFinal.vcxproj y .filters: integración en Visual Studio y sus carpetas visuales.
- CMakeLists.txt: incluye PlanetaOoo.cpp para conservar la compilación del equipo con CMake.

## Validación

Release x64 compiló con Visual Studio Insiders. El ejecutable terminó con código 0 y generó 31 capturas con glGetError=0: 22 vistas anteriores, siete superficies, Ooo de noche y Ooo en el espacio. Se inspeccionaron visualmente las tres nuevas vistas de Ooo. La navegación por P conserva la lógica original y ahora recorre siete entradas. No se realizó una prueba manual del teclado.

Finn, edificios, criaturas, luces propias y skybox personalizado quedan para etapas posteriores. El avatar actual sigue siendo Tizón. Esta etapa es el registro y el piso base, no el planeta final ambientado.
