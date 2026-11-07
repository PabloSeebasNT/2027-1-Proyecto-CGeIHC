# Jake y BMO hechos con codigo

Las versiones importadas se retiraron del dibujo y de la carga de la escena. Sus archivos OBJ, MTL, textura y Blender se conservan en disco para poder recuperar el trabajo anterior. Los personajes que se ven ahora se construyen con geometria C++ y la textura blanca existente.

AmigosCodigoOoo.h/.cpp contiene las figuras y su jerarquia:
- Jake: cuerpo redondeado, orejas caidas, ojos con borde negro, hocico, nariz, boca, lengua, cola, brazos, manos y pies.
- BMO: carcasa con esquinas redondeadas, marco y pantalla, rostro, cruceta, botones, ranuras y rejillas laterales.
- Torso -> hombro -> codo -> mano; torso -> cadera -> rodilla -> pie. Las matrices de los hijos heredan los movimientos del padre.
- Parpadeo, balanceo de brazos y movimiento de cola de Jake.

PersonajesOoo.cpp conserva los recorridos separados alrededor de la casa, velocidades, pausas, orientacion y colisiones. Los pasos siguen la distancia recorrida. El apoyo de los pies se calcula usando la geometria de las nuevas figuras; ya no se leen vertices de archivos OBJ.

No hay que descargar ni exportar modelos para modificar estas versiones. Colores, proporciones y amplitudes estan en AmigosCodigoOoo.cpp; las rutas siguen en PersonajesOoo.cpp.

Validacion: compilacion Release/x64 y cuatro capturas dentro del proyecto (Jake, BMO, paseo y noche), todas con glGetError=0. Revisados ambos personajes y su pose de caminata.

Para verlos: cerrar la ejecucion anterior, abrir ProyectoFinal.slnx, seleccionar Release/x64, compilar y ejecutar Ctrl+F5. Viajar a Ooo: Jake rodea la casa y BMO pasea en el jardin delantero.
