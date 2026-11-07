# Base de Ricardo en Visual Studio Insiders y plan de integración

## Organización

Ubicación propuesta: `C:\Users\urixb\OneDrive\Documentos\labCG\labCG\ProyectoFinal_CGeIHC`.
Es una carpeta hermana de la carpeta que contiene las prácticas. Tiene su propia solución y sus propias copias de las bibliotecas. No se agrega ningún archivo a la solución de las prácticas.

El código y los recursos originales de Ricardo se conservan. Se agregaron `ProyectoFinal.vcxproj` y `ProyectoFinal.slnx`, configurados con v145, C++17, UTF-8 y x64, usando las bibliotecas existentes de tus prácticas. La compilación copia DLL, texturas, modelos y shaders junto al ejecutable.

## Abrir y ejecutar

1. Abre Visual Studio Insiders.
2. Archivo > Abrir > Proyecto o solución: selecciona `ProyectoFinal.slnx` de esta carpeta. También puedes abrir directamente `ProyectoFinal.vcxproj`.
3. Selecciona `Release` y `x64` en la barra superior.
4. Si hace falta, clic derecho en ProyectoFinal > Establecer como proyecto de inicio.
5. Compilar > Compilar solución (`Ctrl+Mayús+B`).
6. Ejecuta sin depuración (`Ctrl+F5`).
7. El resultado está en `bin\x64\Release\ProyectoFinal.exe`. Conserva junto a él las DLL y carpetas de recursos.

No es necesario crear un proyecto vacío, copiar los archivos dentro de una práctica o configurar CMake. Los 17 CPP de Ricardo ya están incluidos, con un solo main. Los futuros CPP deben agregarse al proyecto mediante Agregar > Elemento existente.

Si al abrir faltan recursos, verifica en Propiedades > Depuración que el directorio de trabajo sea `$(OutDir)`. Si faltan includes o bibliotecas, confirma que la carpeta conserva `include`, `glm` y `lib`, y que estás usando x64.

## Recorrido inicial

- 1: avatar; W/S avanza/retrocede, A/D y ratón giran.
- 2: cámara de la carrera provisional.
- 3: nave automática entre planetas.
- 4: recorrido por cohete provisional, estación y cuerpos celestes.
- 5: cámara libre.
- P: cambiar de planeta; los otros planetas todavía tienen pisos provisionales.
- H: adelantar día/noche; L: linterna; R: reflectores de Pyros.
- E: erupción; T: despegue/regreso de nave; V: evolución; Esc: salir.

## Inventario real de Finn

Origen: `C:\Users\urixb\OneDrive\Desktop\modelofinn`.
Hay diez OBJ, diez MTL y una textura `Finn.png`. Todas las piezas tienen UV y normales, y todos los MTL referencian `Finn.png`.

Las piezas son `cuerpofinn`, `cabezafinn`, `brazosupder`, `brazoinfder`, `brazosupizq`, `brazoinfizq`, `piernasupder`, `piernainfder`, `piernasupizq` y `piernainfizq`.

Las coordenadas conservan la posición en el conjunto: Y va aproximadamente de 0.596 a 73.348. Para medir unos 3 metros/unidades, una escala inicial es 3 / (73.348 - 0.596), aproximadamente 0.04124. Antes de escalar, se puede restar 0.596 a la base vertical para apoyar los pies en el piso. La orientación frontal se confirmará visualmente.

El cuerpo tiene geometría que baja hasta Y=5.09, bastante por debajo del torso. Es necesario inspeccionarla antes de decidir si contiene accesorios que deben acompañar al torso o piezas que requieren separación; los límites numéricos no permiten identificarla por sí solos.

## Plan por etapas

### 1. Conservar una base comprobada

Ejecutar Ricardo sin cambios de escena. Verificar cámaras, cambios de planeta, iluminación y animaciones. Registrar esta base en la rama de trabajo Uri antes de integrar; esta carpeta preparada es una copia, no un clon Git y no contiene el historial del equipo.

### 2. Reservar la zona de Hora de Aventura

Elegir con el equipo una de las entradas por asignar en `PLANETAS`. Propuesta: reutilizar provisionalmente el planeta verde (índice 3, superficie en X=13000), sin asumir que esté asignado a Uri.

Crear `PlanetaOoo.h/.cpp`, con funciones de carga, actualización, luces, superficie y esfera espacial. Usar coordenadas locales respecto al centro de superficie, como Pyros. Registrar nombre/responsable, punto de aparición, radio caminable y skybox. Separar las coordenadas de la esfera espacial de las de su superficie.

Puntos de integración en ProyectoFinal.cpp: tabla PLANETAS; carga inicial; actualización del ciclo principal; selección de luces; dibujo de superficies y de planetas en el espacio. Mantener intactas las rutas de cámaras existentes.

### 3. Ensamblar Finn quieto

Crear `Finn.h/.cpp`. Copiar los OBJ y MTL a `Models/Finn/` y `Finn.png` a `Textures/Finn.png`, porque el cargador actual busca ahí las texturas aunque el modelo esté en una subcarpeta.

Cargar diez objetos Model una sola vez. Dibujar todos con la misma matriz raíz al principio: las posiciones exportadas ya ensamblan el conjunto. Revisar textura, escala, orientación, huecos y posibles solapamientos. No centrar cada pieza de forma independiente sin conservar su desplazamiento original.

Estos OBJ separados permiten construir la jerarquía manualmente sin necesitar soporte de esqueletos GLB. El plan no depende de importar las animaciones originales del modelo.

### 4. Articular por matrices padre-hijo

Jerarquía propuesta:

```text
raíz de Finn
└── torso
    ├── cabeza
    ├── brazo superior derecho → antebrazo derecho
    ├── brazo superior izquierdo → antebrazo izquierdo
    ├── muslo derecho → pierna inferior derecha
    └── muslo izquierdo → pierna inferior izquierda
```

Con vértices conservados en coordenadas del personaje, usar para un brazo `Mraíz · T(hombro) · R(hombro) · T(-hombro)`; para su antebrazo, añadir al resultado `T(codo) · R(codo) · T(-codo)`. Así el codo hereda el giro del hombro. Los pivotes deben estar expresados en el mismo espacio del OBJ; no combinar esta fórmula con desplazamientos locales recalculados sin ajustar las matrices.

Alturas aproximadas útiles para localizar articulaciones: hombros Y≈55, codos Y≈43, caderas Y≈33 y rodillas Y≈18.55. Son referencias iniciales, no pivotes verificados. Medir X/Z y confirmar visualmente cada articulación.

### 5. Animar y comprobar

Empezar con un brazo y su codo. Luego caminar con brazos/piernas opuestos, flexión de rodillas y pequeña oscilación del torso. Usar el tiempo transcurrido para independizar la velocidad de los FPS. Añadir una acción por teclado libre, por ejemplo saludo con F, después de reservarla con el equipo. Las teclas E/T/V/L/R/P/H y 1–5 ya tienen funciones.

Finn se propone como personaje secundario jerárquico; conservar Tizón como avatar principal no humanoide exigido en la rúbrica.

### 6. Completar el planeta y la temática

Escenario mínimo propuesto: piso recorrible, casa del árbol, una zona del Dulce Reino, Finn y criaturas propias del planeta. Es una propuesta visual, no contenido ya implementado. Añadir una representación visible de evolución/adaptación en tres etapas y relacionarla con viajes espaciales: Finn por sí solo no satisface ese requisito.

Agregar seis caras de skybox propias, luces puntuales nocturnas y colisiones sencillas con obstáculos. Reutilizar el ciclo de 120 segundos; no crear un segundo reloj independiente. Respetar el presupuesto actual por zona: diez luces puntuales y ocho spotlights, contando la linterna.

### 7. Integrar y documentar

Comprobar que P permite llegar al planeta y volver a Pyros; que las cámaras 1–5 conservan su comportamiento; que Finn mantiene articulaciones unidas; que las texturas funcionan también al ejecutar el EXE; y que la iluminación permite ver el personaje de día y de noche.

Guardar evidencia y bitácora de cada etapa. Preparar commits separados para escena base, carga de Finn, jerarquía, animación y ambientación. No reutilizar elementos de prácticas como evidencia del proyecto final, según los lineamientos.

## Alcance actual

La preparación de Visual Studio no implementa todavía Hora de Aventura ni modifica el cargador. Las piezas originales de Finn no se han editado. Esta guía permite integrar por etapas sin confundir configuración de compilación con funcionalidad terminada.
