# Estacion espacial y atraque

La estacion reemplaza los cilindros y conos provisionales en la misma posicion (-62, 0, 51). Se construye con las primitivas existentes: modulo central, torre de observacion, antena, dos paneles solares y dos hangares con cubierta parcial, plataforma y marcas de atraque. Reutiliza metal.png y plain.png. Dos luces puntuales se encienden de noche; las senales tienen una emision tenue.

## Como verla

Abrir ProyectoFinal.slnx en Visual Studio Insiders, seleccionar Release / x64 y ejecutar con Ctrl+F5. Pulsar 4: primero se ve el lanzamiento y, nueve segundos despues, la toma de la estacion durante diez segundos. Pulsar 5 durante esa toma conserva la posicion y permite quedarse observando con la camara libre. Se ajusto solamente el encuadre de la toma de la estacion; las demas tomas y teclas permanecen iguales.

## Movimiento

EstacionEspacial.cpp dibuja dos instancias de la nave existente con turnos separados 25 segundos. Cada ciclo dura 50 segundos:

- 0 a 14: aproximacion y desaceleracion hacia el hangar.
- 14 a 18: descenso hasta apoyar el tren.
- 18 a 22: espera en la plataforma.
- 22 a 26: elevacion ligera, plegado del tren y giro de salida.
- 26 a 30: ascenso a la altura de salida.
- 30 a 44: salida y alejamiento.
- 44 a 50: giro en el exterior para repetir la aproximacion.

Cada tramo utiliza S(u)=6u^5-15u^4+10u^3, con u limitado entre 0 y 1. La velocidad y la aceleracion de esa interpolacion son cero en los extremos. Se evalua por tiempo de escena, sin sumar desplazamientos por cuadro. Las posiciones coinciden en las uniones y al repetir el ciclo. Los hangares tienen carriles separados y no hay simulacion de colisiones. El apoyo se calcula como altura de cubierta (0.6) + altura del tren (2.2) por escala (0.9) = 2.58.

La segunda instancia es morada. DibujarNaveVerde tiene un parametro opcional morada=false; solo la nave del segundo hangar lo activa. Cambia el acabado del casco y las alas por metal tintado morado, conservando geometria, cabina, motores, radar y tren. Las llamadas anteriores siguen usando el aspecto verde, incluidos Pyros y la carrera.

## Archivos y verificacion

Nuevos: EstacionEspacial.h/.cpp. Integracion: ProyectoFinal.cpp, NaveVerde.h/.cpp, ProyectoFinal.vcxproj, su archivo filters y CMakeLists.txt. No se modificaron los modulos de Ooo, Pyros, CircuitoEspacial ni CarreraEspacial.

Se compilo Release x64 con MSBuild de Visual Studio Insiders y se ejecuto el programa con cinco capturas: general, atraque, camara 4, noche y carrera. El proceso termino con codigo 0; las capturas reportaron glGetError = 0. Se inspeccionaron visualmente las vistas de la estacion. No se compilo con CMake ni se midieron FPS.

Los lineamientos oficiales, paginas 1 y 2, piden una estacion de entrada y salida de naves, geometria 3D texturizada y que la estacion aparezca en el cuarto recorrido. Esta integracion cubre esa parte; no constituye una revision completa de la rubrica del equipo.
