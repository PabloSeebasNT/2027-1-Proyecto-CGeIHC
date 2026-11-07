Lady Arcoiris: vuelo jerarquico en Ooo
====================================
Codigo: ArcoirisOoo.cpp / ArcoirisOoo.h.
Modelos y texturas originales conservados.

Jerarquia
cuerpo1 -> cuerpo2 -> cuerpo3 -> cuerpo4 -> cuerpo5 -> cola
cuerpo1 -> caebza -> ojosboca
cuerpo1 -> patas delanteras; cuerpo5 -> patas traseras.

1. Recorrido(tiempo) calcula una elipse sobre el lago y el jardin.
   Centro (12,26), radios 26 y 31, una vuelta de unos 114 segundos.
   Altura de la raiz: 7.4 unidades con variaciones suaves; escala 4.
   La direccion se obtiene de dos posiciones cercanas de la ruta.
   La matriz raiz aplica posicion, rumbo, elevacion, inclinacion y escala.
2. Giro(parte,fase) genera una onda senoidal de 10 grados y periodo de
   4.33 segundos. Cada segmento recibe un desfase de 0.85 radianes.
   Se resta el angulo del segmento anterior para que la cadena no se enrolle.
3. CalcularParte recorre la jerarquia. Cada hijo hereda la matriz de su padre:
   matrizPadre * trasladar(pivoteHijo-pivotePadre) * rotacionesLocales.
   Para dibujar el OBJ se aplica despues trasladar(-pivoteHijo).
4. DibujarCuerpo mezcla las matrices vecinas en una franja de 0.14 unidades
   a cada lado de las uniones. Los vertices compartidos reciben los mismos
   pesos, conservando el cierre del cuerpo cuando se dobla.
   Solo se actualizan los vertices de las cinco piezas del cuerpo; las
   texturas y los buffers se cargan una vez, no se crean en cada cuadro.
5. La cabeza compensa parte de la oscilacion; cara, patas y cola heredan
   sus padres. Las patas alternan un movimiento pequeno y la cola lleva
   retraso respecto al cuerpo.

La animacion depende del tiempo de escena, no del numero de cuadros.
Comienza automaticamente y solo se dibuja en Ooo. No requiere teclas.
Es un circuito definido, no navegacion automatica entre obstaculos nuevos.

Para ajustar: Recorrido controla ruta y altura; Giro controla amplitud,
ritmo y desfases. La escala permanece en DibujarArcoirisOoo.
Validacion: compilacion Release x64 y capturas en t=0, 1.6, 30 y 60 segundos,
sin errores OpenGL. No se modificaron los otros personajes ni sus recorridos.

Correccion de patas y acrobacia
------------------------------
Se detectaron tres fragmentos independientes dentro de cuerpo4/cuerpo5:
11 vertices de pata derecha, 7 de izquierda y otros 3 de derecha.
Ahora se transforman con el hueso de su pata, sin eliminar geometria ni
alterar los OBJ originales. Quedan excluidos de la mezcla del cuerpo.

EvaluarAcrobacia controla un tonel (giro longitudinal completo) de 7 segundos.
Comienza en el segundo 18 de cada ciclo de 32 segundos. Usa interpolacion
quintica para empezar y terminar sin cambios bruscos de velocidad de giro.
Durante el giro sube hasta 1.6 unidades adicionales y recoge las patas;
la amplitud de la onda baja de 10 a 7 grados y luego se recupera.
La altura habitual de la raiz subio de 5.2 a 7.4 unidades.

Validacion adicional: Release x64; vistas inferiores de las patas y capturas
al inicio, durante, invertida y al terminar el tonel (18,20,21.5,25 segundos).
Sin errores de OpenGL en las capturas.