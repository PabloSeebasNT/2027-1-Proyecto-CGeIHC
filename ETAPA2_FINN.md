# Etapa 2: Finn estatico en Ooo

Se cargan los diez OBJ y sus diez MTL desde Models/Finn. Finn.png esta en Textures, donde la busca el cargador existente. Los recursos son copias sin cambios de los archivos de modelofinn.

Finn.h/Finn.cpp cargan cada pieza una sola vez y las dibujan con una matriz raiz comun. Se conservan sus coordenadas exportadas para evitar desensamblarlas. No se aplican animaciones ni giros independientes; los pivotes quedan para la etapa 3.

Altura: 3 unidades. Escala: 3/(73.348-0.596), aproximadamente 0.04124. Se descuenta la altura minima original de los pies (0.596) antes de aplicar la escala para apoyarlos en el plano Y=0. Posicion relativa al centro de Ooo: (4,0,22), mirando hacia +Z, frente al punto de llegada del avatar y a su derecha.

PlanetaOoo.cpp llama a CargarFinn al cargar la zona y a DibujarFinn al dibujar su superficie. Finn.cpp se agrego tanto a Visual Studio como a CMake. El cargador compartido Model.cpp no se modifico.

## Como verlo

Recargar la solucion en Visual Studio si lo solicita. Release / x64; compilar y ejecutar con Ctrl+F5. Desde Pyros, presionar y soltar P seis veces. Finn aparece delante y a la derecha. Acercarse con W y orientar el avatar con A/D o raton. La camara 5 permite observarlo desde otros angulos. H adelanta el ciclo dia/noche y L activa la linterna del avatar.

## Verificacion

Compilacion Release x64 correcta. Ejecucion con codigo de salida 0, sin mensajes de fallo de carga. Se generaron seis vistas (llegada, frente, espalda, lado, noche y Pyros), todas con glGetError=0. Se revisaron visualmente frente, espalda, lado y llegada; la textura facial, ropa y mochila se ven en su posicion. No se realizo una prueba manual del teclado.

Las capturas de Evidencias/26_finn_frente.png y 27_finn_llegada.png corresponden a esta version.
