Evolucion redonda de Ooo
=======================
J alterna las tres etapas en Ooo; transicion gradual de 2.8 segundos.
Ubicacion: claro al final del sendero derecho, local (43,-33).

Etapa 1: cria azul, tamano 1.18 y cuerno de longitud local .40.
Etapa 2: misma silueta redonda, tamano 1.52 y paleta lavanda.
Etapa 3: tamano 1.78, paleta turquesa, cuerno local .66 y pequenas particulas
que salen de su punta. Conserva las franjas de color para reconocer la especie.
Se retiro el cuerpo alargado segmentado de la version anterior.

Animacion: balanceo del cuerpo, flotacion en la tercera etapa, hombros y codos
con fases distintas, movimiento de pies y parpadeo. Todo depende del tiempo
de escena. Jerarquia: raiz -> cuerpo -> rostro/cuerno y hombro -> codo -> mano.
Particulas: hasta 18 esferas pequenas; edad calculada con modulo del tiempo,
trayectoria en espiral y tamano senoidal que nace y desaparece suavemente.
No se crean objetos nuevos por cuadro ni se altera el sistema de luces.

Mezclar interpola tamano, madurez de color, longitud del cuerno y poder.
El control de estados y la proteccion frente a pulsaciones repetidas se conservan.

Verificacion de esta revision: compilacion Release x64 y seis capturas reales
(tres etapas, dos transiciones y noche), sin errores de OpenGL.
No se repitio en esta revision la prueba automatica de matrices de la version
alargada ni se hizo un recorrido manual. Sus resultados anteriores no deben
presentarse como pruebas de esta nueva geometria con particulas.

Abrir ProyectoFinal.slnx en Visual Studio Insiders, Release/x64, Ctrl+F5.
Con P llegar a Ooo, seguir el sendero derecho y pulsar J.

Revision del caparazon azul y alas
---------------------------------
Etapas 2 y 3: domo azul situado en la espalda (-Z), borde claro y placa
hexagonal central. Las costuras se proyectan sobre la superficie elipsoidal.
Etapa 3: alas celestes con plumas claras, unidas a los laterales del caparazon.
Cada ala hereda su matriz y gira sobre un pivote con sin(t*3.5).
El crecimiento utiliza los factores interpolados caparazon y alas existentes.
Se conserva la forma redonda, los colores del cuerpo, el cuerno y las particulas.
Pruebas de esta revision: compilacion Release x64 y tres capturas reales
(etapa 2 posterior, etapa 3 posterior y frontal), glGetError=0.
Control sin cambios: J en Ooo, transicion de 2.8 segundos.
