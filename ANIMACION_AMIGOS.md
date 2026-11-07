# Paseos de Jake y BMO

Jake recorre un circuito amplio alrededor de la casa a 1.3 unidades por segundo. BMO pasea por el jardin delantero a 0.85 unidades por segundo. Los puntos de control se encuentran en CargarPersonajesOoo, dentro de PersonajesOoo.cpp.

Los caminos usan curvas Catmull-Rom muestreadas y una tabla de distancia acumulada. Asi la velocidad no depende de la separacion entre puntos. La direccion se calcula mirando un poco por delante y por detras de la posicion actual. Las rutas se corrigen contra las colisiones del paisaje con margen para el cuerpo.

En cada media vuelta frenan durante 1.2 segundos, descansan (Jake 3.2 segundos, BMO 4) y vuelven a acelerar. Durante la pausa miran alrededor. El tiempo es el de la escena, por lo que el resultado no depende de la cantidad de cuadros por segundo y las capturas pueden reproducirse.

La fase de las piernas depende de la distancia recorrida. BMO balancea los brazos de forma contraria a las piernas. Jake mueve sus piernas y la cara; sus brazos siguen unidos al cuerpo, tal como se conservo el modelo al importarlo. Las amplitudes disminuyen al frenar. Un balanceo lateral pequeno acompana la caminata. Las articulaciones conservan sus matrices padre-hijo.

Para apoyar los pies se calcula el vertice mas bajo de las piernas transformadas, y se ajusta la altura del personaje sobre el pasto o el sendero. No es un sistema de huesos con rodillas ni cinematica inversa: trabaja con las piezas rigidas disponibles.

Las colisiones de los personajes siguen sus posiciones actuales. Los circuitos finales estan separados (6.84 unidades minimas entre las curvas muestreadas antes de la correccion de obstaculos). No se cambiaron los controles de Finn ni las luces o cielos.

Validacion: compilacion Release/x64; capturas de varios momentos del paseo y primeros planos de dos fases de paso, de dia y de noche, con glGetError=0. Prueba visual realizada mediante capturas, no mediante una sesion manual de teclado.

Para verlos: cerrar la ejecucion anterior, abrir ProyectoFinal.slnx, compilar Release/x64 y ejecutar Ctrl+F5. En Ooo caminan automaticamente; buscar a Jake por los costados de la casa y a BMO en el jardin delantero.
