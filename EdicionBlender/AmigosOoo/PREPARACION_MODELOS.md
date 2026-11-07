# Jake y BMO en Ooo

Se prepararon realmente en Blender 5.2 mediante un script reproducible. Los archivos originales del usuario no se modificaron.

## Resultado

| Modelo | Triangulos originales | Triangulos finales | Piezas | Altura en Ooo |
|---|---:|---:|---:|---:|
| Jake | 12562 | 9286 | 4 | 2.5 |
| BMO | 2581 | 2479 | 7 | 1.7 |

Finn mide 3.0 unidades. Jake queda en (-1,23) y BMO en (9,24), coordenadas locales de Ooo, junto al punto inicial de Finn. Los pies apoyan sobre el sendero. Se agregaron colisiones circulares alrededor de sus cuerpos.

## Trabajo en Blender

Jake: correccion de orientacion, conexion y empaquetado de su textura original, separacion por partes desconectadas, reduccion con Decimate (0.68 en cuerpo, 0.78 en cara y piernas), union de vertices coincidentes a tolerancia pequena, normales recalculadas, escala y base normalizadas. Reduccion del 26.1% de triangulos. Cara y piernas se conservan como hijos del cuerpo. Los brazos y la cola siguen integrados en el cuerpo: no se recortaron para evitar costuras. No incluye esqueleto de huesos ni articulaciones de codos.

BMO: el FBX tenia relaciones y transformaciones que dejaban piezas desacomodadas, y referencias a texturas externas ausentes. Se reconstruyo el ensamblaje con la geometria original; se separaron brazos y piernas por islas; se recolocaron botones; se usaron materiales planos para recuperar sus colores. La pantalla venia vacia: se anadieron dos ojos y una sonrisa ligera. Se quitaron las letras laterales descolocadas de la version de juego, conservadas en el original. No se aplico una reduccion agresiva porque ya era un modelo ligero. El balance final, incluyendo el rostro nuevo, es 4.0% menos triangulos.

Las texturas ausentes de BMO no se recuperaron del archivo: se sustituyeron por materiales de color. Jake conserva el PNG incluido en su ZIP.

## Jerarquia

Jake_raiz -> cuerpo -> cara, pierna izquierda, pierna derecha.
Bmo_raiz -> cuerpo -> botones, cara, brazo izquierdo, brazo derecho, pierna izquierda, pierna derecha.

PersonajesOoo.cpp usa un indice de padre, un pivote y una matriz acumulada por pieza. Primero hereda la transformacion del padre, luego aplica el desplazamiento y el giro propios. El giro esta en cero: esta entrega integra los modelos en reposo, no incorpora controles ni animacion. Los pivotes coinciden con los archivos Blender.

## Archivos

- PersonajesOoo.h/.cpp: carga, jerarquia, posiciones y colisiones.
- Models/Ooo/Amigos: once OBJ con sus MTL.
- Textures/Jake_BaseColor.png: textura de Jake.
- EdicionBlender/AmigosOoo: archivos originales copiados, .blend preparados, scripts y este informe. inspeccionar.py genera los originales .blend; preparar.py reconstruye y exporta los modelos. Los scripts se ejecutan con Blender en segundo plano desde esa carpeta.

Validacion: compilacion Release/x64 y cuatro capturas dentro del proyecto (grupo, Jake, BMO y noche), todas con glGetError=0. Revisados materiales, proporciones y apoyo. No se ha probado manualmente un recorrido con teclado.

Para verlos: abrir ProyectoFinal.slnx en Visual Studio Insiders, aceptar recarga si se solicita, compilar Release/x64 y ejecutar Ctrl+F5. Viajar a Ooo con P; estan cerca de Finn al llegar.
