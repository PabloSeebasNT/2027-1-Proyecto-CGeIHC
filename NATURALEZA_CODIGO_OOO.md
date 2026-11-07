Naturaleza de Ooo construida por codigo
======================================
Archivo: OooModelosNature.cpp. El nombre se conserva para mantener las llamadas
existentes, pero ya no usa Model ni LoadModel ni archivos Models/OooNature.
Los modelos importados anteriores permanecen archivados, sin cargarse aqui.

CrearTroncoCurvo: anillos de 12 vertices, radio que disminuye con la altura,
centro desplazado para curvar el tronco y base ensanchada. Ramas y raices
heredan la matriz del arbol.
CrearForma/PuntoForma: coordenadas esfericas con deformaciones senoidales.
Copas: normales suaves y tres variantes reutilizadas. Dos siluetas de arbol,
una redondeada y otra alta de copa colgante; tonos, giro y escala variables.
Arbustos: tres volumenes de diferentes tamanos, ligeramente enterrados.
Rocas: malla de pocos triangulos, normales por cara y base plana en Y=0.
La malla se construye una sola vez y se reutiliza mediante transformaciones.

Se mantienen 42 arboles, 38 ubicaciones de arbustos, 16 grupos de rocas y
piedras pequenas. Las posiciones y las colisiones existentes se conservan.
Las texturas de corteza, hojas y piedra de Textures/OooEtapa1 se mantienen;
crear geometria por codigo no significa eliminar las texturas.
La casa del arbol, los personajes y sus recursos quedan sin cambios.

Para estudiar y adaptar: revisar PuntoForma (silueta), CrearTroncoCurvo
(anillos), ArbolCodigo (jerarquia local) y las tablas de posiciones.
Esta sustitucion no garantiza una calificacion ni atribuye autoria del
codigo asistido al alumno. Revisar los criterios de asistencia del curso.

Validacion: compilacion Release x64 y dos capturas dentro del programa,
sin errores OpenGL. Falta la prueba manual completa de recorrido del alumno.
