# Princesa Grumosa: NPC de Ooo

Personaje creado con geometria en C++, inspirado en la referencia proporcionada. No usa un OBJ/FBX ni una textura nueva: emplea la textura blanca existente y colores de material.

GrumosaOoo.cpp contiene:
- Cuerpo: una sola malla parametrica redondeada. Circulos definen los bultos del contorno; el frente se suaviza hacia una elipse para evitar pliegues radiales. Normales calculadas a partir de la superficie.
- Estrella: diez vertices alternados, centro elevado y borde con grosor, inclinada sobre la frente.
- Ojos, reflejos, boca, lengua, dientes y manos: esferas escaladas.
- Brazos: tubos sobre curvas Bezier cubicas. Cada brazo gira sobre su hombro y hereda la matriz del cuerpo.
- Animacion ambiental: flotacion, balanceo, pequenos giros, gestos de brazos y parpadeo cada 5.4 segundos.

Su posicion es (25,28) en coordenadas locales de Ooo, en el jardin a la derecha del lago. Tiene colision circular para el avatar y Finn. Es un NPC ambiental: no tiene dialogo ni misiones. Se dibuja exclusivamente en Ooo.

Ajustes sencillos: POSICION mueve el NPC; MORADO cambia su color; los lobulos de Radio ajustan su silueta; DibujarGrumosaOoo controla la animacion y la cara. Las mallas se crean una sola vez al cargar.

Integracion: GrumosaOoo.h/.cpp, llamadas en ProyectoFinal.cpp y colision en OooPaisaje.cpp. Registrado en Visual Studio y CMake.

Verificacion: compilacion Release/x64 y tres capturas (frontal, lateral y noche con parpadeo), sin errores OpenGL. Para verlo, cerrar la ejecucion anterior, compilar y ejecutar ProyectoFinal.slnx con Release/x64; viajar a Ooo y acercarse al lado derecho del lago.
