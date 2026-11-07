# Mayordomo Menta - NPC de Ooo

Creado con geometria en C++ a partir de la referencia proporcionada. No se importa un modelo externo ni se genera una textura nueva.

MentaOoo.cpp construye el caramelo con una esfera achatada, franjas rojas proyectadas en ambas caras, un casquete azul para el traje, triangulos para el mono, camisa y faldones, tubos curvos para brazos y piernas, y esferas escaladas para ojos, manos y zapatos. Todo usa la textura blanca existente y colores de material.

El NPC esta en (17,31) respecto al centro de Ooo, cerca de Grumosa y a la derecha del lago. Tiene colision circular. La animacion incluye respiracion leve, parpadeo y un gesto corto con el brazo derecho cada nueve segundos; los pies permanecen apoyados. Los brazos heredan la matriz del cuerpo y giran sobre sus hombros. Es un NPC ambiental, sin dialogos ni misiones.

Ajustes: POS cambia su ubicacion; AZUL, ROJO y BLANCO cambian colores; CrearFranjas controla los gajos del caramelo; DibujarMentaOoo controla proporciones y animaciones.

Archivos: MentaOoo.h/.cpp, llamadas en ProyectoFinal.cpp y colision en OooPaisaje.cpp. Incluido en Visual Studio y CMake.

Verificado: compilacion Release/x64 y tres capturas (frente, costado y noche con parpadeo), todas con glGetError=0. Para verlo, cerrar la ejecucion anterior y ejecutar con Ctrl+F5 en Visual Studio Insiders, luego viajar a Ooo.
