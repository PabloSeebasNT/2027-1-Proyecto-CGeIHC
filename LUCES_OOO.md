# Luces de Ooo

Tres faroles: dos sujetos a la fachada y uno sobre un poste junto al sendero y al lago. Estan construidos con cubos, cilindro y cono. Su iluminacion usa PointLight igual que practica9full.cpp: color, intensidad ambiente y difusa, posicion y atenuacion constante, lineal y cuadratica.

Siete grupos de ocho luciernagas (56 en total) vuelan cerca de arboles existentes. Cada bicho tiene altura, fase y pulso diferentes. El punto luminoso tiene emision y un halo aditivo. Cada grupo comparte una luz puntual para iluminar suavemente el pasto: tres faroles + siete grupos respetan el limite de diez luces del shader. No hay una luz puntual por insecto.

Encendido usa la misma variable hora que el sol y los skyboxes. La intensidad aumenta gradualmente alrededor del atardecer y baja al amanecer. De dia no se envian las luces de Ooo ni se dibujan luciernagas. Todo se activa solo cuando la camara esta en Ooo.

Archivos principales: OooLuces.h/.cpp. La funcion PuntoCasaArbolOoo en OooCasaArbol permite colocar los faroles siguiendo la posicion, giro y escala de la casa. ProyectoFinal conecta la carga, las luces y el dibujo; los halos se dibujan al final y restauran el estado de mezcla y profundidad.

Para ajustar: CALIDO y BICHO controlan colores; grupos controla las posiciones; POR_GRUPO la cantidad de insectos; PosicionBicho su movimiento; Pulso su parpadeo; LucesOoo las intensidades y el alcance. Si aumentas grupos o faroles, revisa el limite MAX_POINT_LIGHTS.

Prueba en Visual Studio Insiders: Release/x64, Ctrl+Shift+B y Ctrl+F5. Viaja a Ooo con P y adelanta la hora con H. Validacion realizada: compilacion y cinco capturas (casa de dia/noche, luciernagas en dos instantes y vista general), sin errores OpenGL. La luz no genera sombras proyectadas: se usa el sistema de iluminacion existente de las practicas.
