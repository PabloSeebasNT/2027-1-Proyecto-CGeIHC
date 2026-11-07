# Circuito espacial

El circuito se encuentra en la zona compartida (centro 7000, 0, 0). Conserva el suelo y la plataforma que ya estaban en esa zona.

## Para verlo

Abre ProyectoFinal.slnx en Visual Studio Insiders, selecciona Release y x64 y ejecuta con Ctrl+F5. Presiona 2 para seguir a la primera nave de la carrera. Las dos naves recorren automaticamente el circuito desde que inicia la escena. La tecla 3 conserva el recorrido espacial original; 5 permite pasar a la camara libre desde la posicion actual.

## Construcción

CircuitoEspacial.h/.cpp contienen la trayectoria cerrada Catmull-Rom, la geometría y las luces. Sus 32 puntos de control generan 1280 tramos, una longitud aproximada de 907.36 unidades y una calzada de 12 unidades de ancho. Las elevaciones van aproximadamente de 5 a 9 unidades sobre el suelo.

La geometría se genera una sola vez y se agrupa en seis mallas por acabado. Incluye carretera metálica, barreras, soportes, marcas de salida, meta a cuadros, flechas y ocho luminarias. Reutiliza las texturas metal.png y plain.png existentes. Las barreras son visuales; todavía no tienen respuesta de colisión.

PoseCircuitoEspacial recibe una distancia recorrida y devuelve posición y dirección; utiliza una tabla de distancias acumuladas para preparar el movimiento de las naves. La distancia se envuelve al completar una vuelta.

ProyectoFinal.cpp carga y dibuja el circuito únicamente en la zona compartida, añade sus luces allí y conecta la cámara 2 a la trayectoria. Se retiró el cubo provisional que representaba la nave. Los archivos de proyecto de Visual Studio y CMakeLists.txt registran el módulo nuevo. No se modificaron los módulos de Ooo, Pyros, Finn ni los NPC.

## Verificación realizada

- Compilación Release x64 con MSBuild de Visual Studio Insiders completada; permanecen advertencias previas de conversión en Window.h.
- Ejecución y cuatro capturas: vista general, salida/meta, curva y noche, con glGetError = 0.
- Validación de índices y valores finitos; cierre de la trayectoria sin separación y sin cruces entre los segmentos de sus bordes en planta.
- Resultado: 31708 triángulos. No se realizó una medición de FPS ni se compiló con CMake.

## Revisión del trazado

Se amplió la vuelta exterior y se agregó un sector interior con curvas en S, inspirado en la referencia del usuario. La salida conserva sus puntos alineados. Se verificaron también los bordes exteriores a 6.6 unidades del centro para descartar cruces de la estructura. La revisión se compiló y ejecutó con cuatro capturas nuevas.


## Naves de carrera

CarreraEspacial.h/.cpp reutilizan dos instancias de DibujarNaveVerde, sin cambiar NaveVerde.cpp ni sus usos en Pyros y el espacio. Es el unico modelo de nave completo encontrado en esta version; las naves provisionales de la estacion permanecen intactas.

Cada corredor tiene carril, velocidad media, posicion inicial y fase. La distancia es salida + velocidad*t + (2/.22)*(sin(.22*t+fase)-sin(fase)). Su derivada da una velocidad siempre positiva con variaciones suaves: no se avanza una cantidad fija por cuadro. La pose se obtiene de la curva Catmull-Rom del circuito y se desplaza 2.6 unidades hacia cada lado. La orientacion usa puntos anteriores y posteriores del propio carril. La nave se inclina un poco en las curvas y flota 1.65 unidades sobre la pista, con una oscilacion vertical de 0.10 unidades. La escala .36 permite que quepan ambas naves.

La camara 2 consulta la misma pose que se usa para dibujar al corredor 0. Mantiene el horizonte vertical para que la inclinacion de la nave no provoque balanceo de la camara. Los motores, radar y tren plegado se dibujan con las funciones existentes. No hay modelos ni texturas nuevos.

Verificacion de esta integracion: compilacion Release x64 con MSBuild de Visual Studio Insiders y ejecucion con cinco capturas (salida, curva interior, siguiente vuelta, noche y camara 3). Todas reportaron glGetError = 0 y el proceso termino con codigo 0. Las capturas son comprobaciones en instantes concretos, no una prueba continua de rendimiento a distintos FPS.

Pendiente si se desea una competencia con reglas: cuenta regresiva, contador de vueltas, clasificacion y ganador. Actualmente es una carrera animada continua con adelantamientos; no incluye conduccion manual ni colisiones fisicas.

## Revision de la camara de carrera

La tecla 2 ya estaba asignada exclusivamente a CAM_CARRERA en Window.cpp. Se mantiene esa asignacion y se agrega amortiguacion a la posicion y al punto de mira en ProyectoFinal.cpp: mezcla = 1 - exp(-8*dt). La posicion deseada esta 10 unidades detras y 5 arriba de la primera nave, con la mirada adelantada 6 unidades. Al activar la camara se reinicia el seguimiento para no arrastrar posiciones de otros modos. El horizonte permanece vertical.

Se ejecuto una comprobacion temporal de la funcion real CalcularCamara durante 120 segundos simulados a 30, 60 y 144 FPS: no hubo posiciones invalidas ni posiciones delante o por debajo de la nave. La altura minima fue 4.088 unidades y la diferencia final entre 30 y 144 FPS fue 0.2372 unidades. La reentrada desde la camara espacial tuvo error cero respecto de la posicion deseada. El codigo temporal de prueba se retiro antes de recompilar la version entregada. No se realizo una prueba manual pulsando teclas; su asignacion y las transiciones se revisaron en codigo. La version final se ejecuto con las cinco capturas de la carrera y camara espacial.
