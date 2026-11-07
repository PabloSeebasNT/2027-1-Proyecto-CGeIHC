# Cielos de Ooo

Se integraron las 18 imagenes originales de skyboxes.zip, sin editar sus dibujos, en Textures/Skybox/Ooo. Sunny_Day corresponde al dia, Golden_Hour al amanecer y atardecer, Moonlit_Night a la noche.

La seleccion usa la misma variable hora que ActualizarSol, no un temporizador aparte. El ciclo existente dura 120 segundos: 0 amanecer, .25 mediodia, .50 atardecer, .75 medianoche. La tecla H sigue adelantando 15 segundos del ciclo completo.

Transiciones suaves:
- 0 a .10: amanecer dorado hacia dia.
- .10 a .40: dia soleado.
- .40 a .50: dia hacia atardecer dorado.
- .50 a .60: atardecer hacia noche.
- .60 a .90: noche.
- .90 a 1: noche hacia el siguiente amanecer.

Golden_Hour se gira 180 grados al amanecer para situar su sol al este (+X); al atardecer permanece al oeste (-X). El contenido dibujado en las imagenes es estatico: el movimiento continuo del sol sigue siendo el de la iluminacion existente.

Solo la zona PLANETA_OOO llama DibujarCieloOoo. Los demas planetas y el espacio mantienen sus skyboxes y su ajuste de brillo. Se corrigio la orientacion de los cubemaps nuevos y su carga RGB, porque dos PNG contienen alpha. No se cambiaron los modelos ni el terreno.

Validacion: compilacion Release/x64 y siete capturas automatizadas (cuatro fases, transicion, cierre del ciclo y Pyros), todas con glGetError=0. Para probar, abrir ProyectoFinal.slnx, compilar Release/x64, ejecutar Ctrl+F5 y usar P para llegar a Ooo; H permite avanzar el ciclo.
