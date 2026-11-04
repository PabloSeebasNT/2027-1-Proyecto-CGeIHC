# Proyecto final CGeIHC 2027-1: base del equipo y planeta Pyros

Temáticas: "Viajes espaciales" y "Evolución de especies intergalácticas".

Esta carpeta tiene la base que comparte todo el equipo (cámaras, ciclo de día y noche, zonas con su skybox, avatar)
y la parte de Ricardo: el planeta Pyros, de lava (universo de Ben 10, mundo de Heatblast), y la nave verde.
Todo lo de esta parte está hecho por código con las figuras de `Geometria.cpp` y texturas generadas con
`herramientas/texturas_proyecto.py`; no se cargó ningún modelo descargado.

Ricardo Emmanuel Galicia Tequianes - 118001740

## Compilar

macOS (Homebrew: glfw, glew, glm, assimp) o Linux (paquetes -dev de las mismas bibliotecas):

    cd ProyectoFinal
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    cd build && ./ProyectoFinal

Windows (para el .exe de la entrega): crear el proyecto como en las prácticas del laboratorio (mismas carpetas de
include y lib de GLEW, GLFW, glm y assimp), agregar todos los .cpp y .h de esta carpeta y copiar `shaders`,
`Models` y `Textures` junto al .exe. No necesita C++17.

## Controles

| Tecla | Acción |
|---|---|
| 1 | Cámara en 3a persona del avatar (plano paralelo a XZ). W/S avanza y retrocede, A/D y ratón giran |
| 2 | Cámara en 3a persona de la nave de la carrera |
| 3 | Cámara en 3a persona de la nave verde volando entre planetas |
| 4 | Recorrido: lanzamiento del cohete, estación espacial, Pyros con su luna y asteroides, panorámica |
| 5 | Cámara libre (W A S D + ratón), para revisar y grabar el video |
| P | El avatar viaja al siguiente planeta |
| H | Adelanta el ciclo de día y noche 15 s |
| L | Linterna del avatar (spotlight) |
| R | Reflectores de la plataforma de Pyros (2 spotlight) |
| E | Erupción del volcán de Pyros |
| T | La nave verde despega de Pyros; con T otra vez regresa y aterriza |
| V | Evolución en el altar: larva -> escarabajo -> coloso -> larva |
| Esc | Cerrar |

## Cómo está organizado el mundo

- **Zona del espacio**, alrededor del origen. Los planetas son esferas acomodadas como en el croquis: x = píxel x - 808,
  z = píxel y - 531. Ahí están el sol, la estación (temporal) y la nave verde, que recorre una spline cerrada entre
  los planetas.
- **Zona de superficie de cada planeta**: su piso está en `centroSuperficie`, a 3000 unidades del siguiente (Pyros en
  x = 4000). El plano lejano de la proyección está en 2000, así que desde un planeta no se ve otro.
- La zona donde está la cámara (`ZonaDe`) decide qué skybox se dibuja y qué luces se mandan al shader. Pyros tiene su
  skybox rojo; los demás usan el del espacio hasta que su dueño agregue el suyo.

## Cómo agrega cada integrante su parte

1. Su planeta ya está en la tabla `PLANETAS` de `ProyectoFinal.cpp` (posición del croquis, color y centro del piso).
   Se cambia el nombre, el responsable y, si tiene skybox propio, se agrega al enum `SKY_...` y se carga con
   `CargarSkybox("nombre")` (caras `nombre_px.png`, `nombre_nx.png`, ... en `Textures/Skybox`).
2. Igual que `Pyros.h`, un archivo con sus funciones: cargar (texturas, modelos, mallas), luces y dibujar la
   superficie. En `main` se llaman donde están las de Pyros, con `if (zona == SU_PLANETA)`.
3. Para dibujar se usa `Dibujar(malla, matriz, color, emision)` o `DibujarModelo(modelo, matriz)`; la textura con
   `UseTexture()` y el material con `UsarMaterial(matBrillante | matOpaco | matMetal)`.
4. Animaciones de teclado: `mainWindow.getPulsada(GLFW_KEY_...)` regresa true una vez por cada vez que se presiona.
   Teclas libres: B, C, F, G, I, J, K, M, N, O, Q, U, X, Y, Z y las flechas.
5. Luces: hasta 10 puntuales y 8 spotlight por zona (`CommonValues.h` y `shader_light.frag`). Pyros usa 9 puntuales
   de noche y 3 spotlight.
6. Lo marcado como **TEMPORAL** en `ProyectoFinal.cpp` (nave de la carrera, plataforma de lanzamiento con su cohete y
   estación con naves que entran y salen) solo sirve para que las cámaras 2 y 4 tengan qué seguir. Quien haga la carrera
   deja `PoseNaveCarrera` con su nave real; quien haga el lanzamiento, `AlturaCohete` y `PLATAFORMA_LANZAMIENTO`;
   quien haga la estación, `POS_ESTACION` y `DibujarTemporalesEspacio`.

## Cambios a la plantilla del laboratorio

- `shader_light.frag`: uniforms `emision` (lava, ojos y cristales brillan de noche), `desplazamientoUV` (lava que corre)
  y `transparencia`. Si no se asignan valen 0 y todo se ve como antes.
- `skybox.frag` y `Skybox`: uniform `brillo` para el ciclo de día y noche; las caras se cargan sin voltear (antes
  quedaban volteadas porque `Texture::LoadTextureA` deja activado el volteo).
- `DirectionalLight::SetDireccion` y `Light::SetIntensidades` para mover el sol.
- `Shader`: getters de los uniforms nuevos. `Window`: teclas del proyecto y `getPulsada`.
- `Shader_light.cpp` incluye `<cstring>` (en Linux y Windows `strlen` no llegaba por otros encabezados).

## Qué cubre esta parte (rúbrica)

- **Avatar**: "Tizón", criatura original de Pyros con cuatro patas, cola y ojos en tallos. Jerarquía cuerpo -> cuello
  -> cabeza -> mandíbula / tallos -> ojos; cuerpo -> 4 patas (cadera, muslo, rodilla, espinilla, pie, garras); cuerpo ->
  cola de 5 segmentos -> llama. Textura de piel y material brillante. Caminata en trote: patas en diagonal juntas, rodilla
  que se dobla al avanzar, cuerpo que sube y se mece, cola en onda.
- **Recorridos**: cámaras 1 a 4 y cambio de cámara con 1-5.
- **Iluminación**: sol con ciclo de 120 s (dirección, color e intensidad) y luna de noche; 6 luminarias puntuales de
  Pyros que se prenden y se apagan juntas al anochecer; spotlights con L y R; skybox distinto en Pyros y en el espacio.
- **Animaciones básicas (teclado)**: erupción (E), despegue y aterrizaje de la nave (T), evolución en el altar (V).
- **Animaciones complejas (continuas)**: nave verde en spline entre planetas y en circuito sobre Pyros; escarabajos
  con marcha en trípode sobre elipses; larvas con onda que viaja por su cuerpo; lava que corre y burbujas del lago;
  humo del volcán; luna, satélite y anillo de asteroides orbitando Pyros.
- **Planeta con evolución de especies**: larva de magma -> escarabajo de lava -> coloso de basalto, vivos en el
  planeta, como estatuas en el museo de la evolución y evolucionando en el altar.
- **NPCs**: 5 larvas, 3 escarabajos y el coloso.
- **Satélites y cuerpos espaciales**: luna Ceniza, satélite con paneles y 70 asteroides alrededor de Pyros.
- **Jerarquía**: avatar, nave verde (alas -> motores, tren -> patas -> pies, radar), seres de Pyros, luminarias y el
  sistema Pyros -> luna -> satélite.

Falta, de otros integrantes: la pista de carreras, la plataforma de lanzamiento con el cohete, la estación espacial
y los demás planetas.

## Archivos

- `ProyectoFinal.cpp`: main, tabla de planetas, zonas, ciclo de día y noche, cámaras, capturas.
- `Geometria.*`: figuras por código, ruido, `Dibujar`, materiales compartidos.
- `Avatar.*`: avatar y su caminata. `NaveVerde.*`: nave verde y rutas spline. `Pyros.*`: planeta Pyros.
- `herramientas/texturas_proyecto.py`: genera todas las texturas y los skybox (`python3 herramientas/texturas_proyecto.py`).
- `Evidencias/`: capturas para los manuales. Se vuelven a generar desde `build` con
  `./ProyectoFinal --capturas ../Evidencias/lista_capturas.txt --salir` (guarda .ppm en `build`).
