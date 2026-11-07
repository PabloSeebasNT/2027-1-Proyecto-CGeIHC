# Casa del árbol y jardín de Ooo

La casa aumenta de escala 2.8 a 3.5 (25%). Su base se coloca en Y=0.055, apoyada en el terreno de Y=0.06. Se toma el tronco como referencia, no la piedra más baja del modelo.

El sendero termina en la entrada calculada a partir de la posición, giro y escala de la casa. Se agregan un lago de contorno irregular, borde de tierra, nenúfares y piedras con variaciones de tamaño y orientación. Las flores se apartan del camino y del lago. El avatar y Finn evitan atravesar el lago mediante la colisión del paisaje.

Archivos: OooCasaArbol.h/.cpp, OooPaisaje.cpp y el nuevo OooJardin.h/.cpp. Se actualizan el proyecto de Visual Studio, sus filtros y CMake.

Para ejecutar: abrir ProyectoFinal.slnx en Visual Studio Insiders, aceptar recargar el proyecto si se solicita, seleccionar Release y x64, compilar con Ctrl+Shift+B y ejecutar con Ctrl+F5. Desde Pyros, pulsar y soltar P seis veces para llegar a Ooo.

Verificación: compilación Release/x64 satisfactoria y cinco capturas automatizadas (vista general, entrada, lago, llegada a Ooo y Pyros), con glGetError=0. Las capturas no sustituyen una prueba manual de recorrido con teclado.
