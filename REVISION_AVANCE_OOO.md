# Revision del avance: Ooo y proyecto completo

Revision del 6 de noviembre de 2026 sobre esta carpeta local. No implica que se hayan revisado nuevas ramas del equipo ni entregas en Classroom. El estado de los documentos y videos externos no se puede confirmar desde esta copia.

## Cambio de ambientacion

- 20 arboles adicionales: 42 en total en la lista del bosque.
- 22 ubicaciones adicionales de arbustos: 38 grupos, algunos con una segunda planta.
- 9 rocas grandes adicionales: 16 en total, ademas de las piedras pequenas existentes.
- Distribucion en grupos alrededor de los claros, con tamanos, especies y orientaciones distintas. Se reutilizan los modelos cargados.
- Arboles y rocas grandes coinciden con las listas de colisiones de OooPaisaje.cpp.
- Compilacion Release x64 y capturas del escenario. Falta una prueba prolongada de recorrido manual y rendimiento en la maquina de entrega.

## Ooo: lo que ya existe

Casa del arbol, lago, senderos, vegetacion, rocas y texturas. Finn separado jerarquicamente y controlado con flechas, con saludo mediante G. Jake, BMO, Grumosa y Menta son NPC con recorridos. Arcoiris tiene piezas jerarquicas, cuerpo ondulante y acrobacia. Hay cielo segun la hora, faroles y luciernagas con luces puntuales vinculadas al ciclo dia/noche. La camara y el avatar principal compartidos permiten recorrer Ooo.

## Ooo: prioridades para cerrar tu parte

1. Hacer visible la tematica de evolucion de especies intergalacticas en Ooo. En esta copia se reconoce claramente Hora de Aventura, pero no encontre una demostracion explicita de evolucion en este planeta. Propuesta: una criatura con tres estados, conectada con una pequena zona de investigacion; acordar con el equipo como justificarla en la rubrica.
2. Dar una conexion visual clara con viajes espaciales. Una zona de llegada o un pequeno transporte tematico ayudarian; es una propuesta de ambientacion, no un requisito literal de tener una plataforma de cohete en cada planeta.
3. Revisar el texturizado de los NPC hechos por codigo: varios usan una textura blanca y colores de material. No asegurar que eso satisfaga la exigencia de todos los elementos correctamente texturizados sin revisar el criterio del profesor. Revisar tambien el aspecto espacial del cielo elegido.
4. Prueba final: puertas y caminos, colisiones, objetos apoyados, vuelo, noche, cambio de planetas y fluidez. Las capturas prueban estados concretos, no todas las interacciones.
5. Preparar tu bitacora real, capturas, explicacion de jerarquias/pivotes/animaciones y contribucion a los manuales y videos.

No hace falta asumir que cada alumno debe entregar tres animaciones basicas y tres complejas: el PDF lo pide al proyecto. Ooo ya aporta varios ciclos continuos, pero la calificacion depende de la demostracion y del criterio docente. Finn es humanoide; el avatar principal no humanoide compartido es el de Avatar.cpp. No presentar a Finn como cumplimiento de ese requisito ni afirmar que Jake ya es controlable: actualmente es NPC.

## Proyecto completo: estado en esta copia

| Requisito | Evidencia y pendiente |
|---|---|
| Dos planetas desarrollados | Pyros y Ooo tienen escenas propias. Las otras cinco entradas siguen usando pisos provisionales en esta copia; no todas son necesariamente necesarias para el minimo, pero hay que integrar lo acordado con el equipo. |
| Avatar jerarquico no humanoide | Implementado en Avatar.cpp, con cuatro patas y cola, texturas y caminata. Revisar comportamiento en todos los destinos finales. |
| Animaciones basicas por teclado | E: erupcion; T: nave; V: evolucion de Pyros. G: saludo de Finn es adicional. Ya existen al menos tres acciones, no hay que inventar tres faltantes. |
| Animaciones complejas continuas | Nave, criaturas y NPC, incluyendo Arcoiris. Documentar tres ejemplos claros y como se calculan. |
| Dia/noche, puntuales, spotlights | Implementados; L conmuta la linterna y R reflectores de Pyros. Revisar materiales y continuidad al cambiar de zona. |
| Carrera espacial | PoseNaveCarrera y DibujarTemporalesSuperficie aun usan una elipse y una nave-cubo. Falta circuito y carrera definitivos. |
| Cohete y plataforma | Existen formas y lanzamiento provisionales; sustituir o completar la escena final. |
| Estacion espacial | DibujarTemporalesEspacio sigue usando la estacion y naves de prueba. Falta integracion final. |
| Cuatro recorridos de camara | La estructura existe con 1-4, mas camara libre 5. Los modos 2 y 4 todavia siguen elementos temporales; probarlos con las escenas definitivas. |
| Satelites y cuerpos celestes | Hay elementos en el espacio/Pyros; incluirlos en la demostracion, no marcarlos como ausentes. |
| Integracion de compilacion | Visual Studio Release x64 compila. CMakeLists.txt no incluye ArcoirisOoo.cpp: actualizar si el equipo usa CMake. LEEME.md tiene datos antiguos sobre recursos, teclas y compilacion; requiere revision. |
| Repositorio | Esta carpeta no se reconoce como repositorio Git. Integrar los cambios en la rama real del equipo y conservar historial autentico. No fabricar commits pasados. |

## Entrega y evidencia

Los lineamientos asignan 60% al escenario y 40% a metodologia. Preparar o confirmar fuera de esta carpeta: propuesta y mapa finales, evidencia de ramas y colaboracion, manual de usuario con capturas, manual tecnico con codigo y conclusiones individuales/grupales y bibliografia, bitacora individual, video demo sin narracion y videoblog con participacion del equipo. Probar una copia del Release con DLL, Models, Textures y shaders en otra ubicacion sin Visual Studio.

El PDF indica entrega el sabado 7 de noviembre al mediodia. Su nota final excluye reutilizar ejercicios de practicas y exige autoria estudiantil de los elementos, exceptuando modelos optimizados y texturas. La ayuda externa no se vuelve autoria propia por cambiar su aspecto: confirmar el uso permitido de asistencia y poder explicar/adaptar el trabajo honestamente.

Fuente: Lineamientos del proyecto final laboratorio de Computacion Grafica e IHC grupos 2 y 3 2027-1.pdf, paginas 1-7. Evidencia local: ProyectoFinal.cpp, Avatar.cpp, Pyros.cpp, Finn.cpp, modulos Ooo, CMakeLists.txt y LEEME.md.
