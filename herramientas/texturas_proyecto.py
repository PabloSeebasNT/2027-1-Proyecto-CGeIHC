"""
texturas_proyecto.py - Proyecto final CGeIHC 2027-1
Ricardo Emmanuel Galicia Tequianes - 118001740

Genera las texturas de Pyros, del avatar, de los NPC, de la nave verde y los dos skybox (espacio y Pyros).
Todas salen de ruido (de valor y de Voronoi), así que son originales y se pueden volver a generar iguales.

    python3 herramientas/texturas_proyecto.py        (desde la carpeta ProyectoFinal)

Las texturas de superficie son periódicas (se repiten sin costura). Las del planeta y la luna son equirectangulares
y las caras del skybox se calculan con la dirección de cada píxel según la convención de los cubemap de OpenGL,
por eso las orillas de caras vecinas coinciden.
"""
import os
import numpy as np
from PIL import Image

SALIDA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Textures")
os.makedirs(os.path.join(SALIDA, "Skybox"), exist_ok=True)


def guardar(nombre, rgb, alfa=None):
    rgb = np.clip(rgb, 0.0, 1.0)
    img = (rgb * 255 + 0.5).astype(np.uint8)
    if alfa is not None:
        a = (np.clip(alfa, 0.0, 1.0) * 255 + 0.5).astype(np.uint8)
        img = np.dstack([img, a])
        Image.fromarray(img, "RGBA").save(os.path.join(SALIDA, nombre))
    else:
        Image.fromarray(img, "RGB").save(os.path.join(SALIDA, nombre))
    print("ok", nombre, img.shape)


def mezcla(a, b, t):
    t = np.asarray(t)[..., None] if np.ndim(t) == 2 else t
    return a + (b - a) * t


def suave(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3 - 2 * t)


# ---------------------------------------------------------------- ruido 2D periódico (para texturas que se repiten)
def ruido_periodico(n, celdas, rng):
    g = rng.random((celdas, celdas)) * 2 - 1
    x = np.arange(n) / n * celdas
    i0 = np.floor(x).astype(int)
    f = x - i0
    s = f * f * (3 - 2 * f)
    i0 %= celdas
    i1 = (i0 + 1) % celdas
    # filas = y, columnas = x
    a = g[np.ix_(i0, i0)]
    b = g[np.ix_(i0, i1)]
    c = g[np.ix_(i1, i0)]
    d = g[np.ix_(i1, i1)]
    sx = s[None, :]
    sy = s[:, None]
    return (a + (b - a) * sx) * (1 - sy) + (c + (d - c) * sx) * sy


def fbm_periodico(n, celdas, octavas, rng):
    suma = np.zeros((n, n))
    amp, total = 1.0, 0.0
    for _ in range(octavas):
        suma += amp * ruido_periodico(n, celdas, rng)
        total += amp
        amp *= 0.5
        celdas *= 2
    return suma / total


def voronoi_periodico(n, k, rng):
    """Regresa F1 y F2 (distancia al punto más cercano y al segundo) en un mosaico que se repite.
    Los puntos van en una rejilla de k x k con un desplazamiento al azar en cada celda; así no quedan dos puntos
    casi encimados (eso dejaba celdas completas rellenas de color)."""
    j, i = np.mgrid[0:k, 0:k]
    p = (np.stack([i.ravel(), j.ravel()], 1) + 0.1 + 0.8 * rng.random((k * k, 2))) / k
    y, x = np.mgrid[0:n, 0:n] / n
    f1 = np.full((n, n), 9.0)
    f2 = np.full((n, n), 9.0)
    for px, py in p:
        dx = np.abs(x - px)
        dx = np.minimum(dx, 1 - dx)
        dy = np.abs(y - py)
        dy = np.minimum(dy, 1 - dy)
        d = np.sqrt(dx * dx + dy * dy)
        f2 = np.where(d < f1, f1, np.minimum(f2, d))
        f1 = np.minimum(f1, d)
    return f1, f2


# ---------------------------------------------------------------- ruido 3D (planeta, luna y skybox)
PERM = np.random.default_rng(7).permutation(256)
PERM = np.concatenate([PERM, PERM])
VALORES = np.random.default_rng(11).random(256) * 2 - 1


def ruido3(p):
    b = np.floor(p).astype(np.int64)
    f = p - b
    s = f * f * (3 - 2 * f)
    x, y, z = (b[..., 0] & 255), (b[..., 1] & 255), (b[..., 2] & 255)

    def h(i, j, k):
        return VALORES[PERM[PERM[PERM[(x + i) & 255] + ((y + j) & 255)] + ((z + k) & 255)]]

    sx, sy, sz = s[..., 0], s[..., 1], s[..., 2]
    x00 = h(0, 0, 0) + (h(1, 0, 0) - h(0, 0, 0)) * sx
    x10 = h(0, 1, 0) + (h(1, 1, 0) - h(0, 1, 0)) * sx
    x01 = h(0, 0, 1) + (h(1, 0, 1) - h(0, 0, 1)) * sx
    x11 = h(0, 1, 1) + (h(1, 1, 1) - h(0, 1, 1)) * sx
    y0 = x00 + (x10 - x00) * sy
    y1 = x01 + (x11 - x01) * sy
    return y0 + (y1 - y0) * sz


def fbm3(p, octavas, escala=2.0):
    suma = np.zeros(p.shape[:-1])
    amp, total = 1.0, 0.0
    for o in range(octavas):
        suma += amp * ruido3(p * (escala ** o) + o * 17.3)
        total += amp
        amp *= 0.5
    return suma / total


def direcciones_equirect(ancho, alto):
    fila, col = np.mgrid[0:alto, 0:ancho]
    fi = np.pi * (fila + 0.5) / alto            # 0 en el polo norte (fila de arriba)
    a = 2 * np.pi * (col + 0.5) / ancho
    return np.stack([np.sin(fi) * np.cos(a), np.cos(fi), -np.sin(fi) * np.sin(a)], -1)


# Caras del cubemap en el orden de OpenGL (+X, -X, +Y, -Y, +Z, -Z); la fila 0 es la de arriba de la imagen
def direcciones_cara(cara, n):
    fila, col = np.mgrid[0:n, 0:n]
    sc = 2 * (col + 0.5) / n - 1
    tc = 2 * (fila + 0.5) / n - 1
    uno = np.ones_like(sc)
    d = {
        "px": (uno, -tc, -sc),
        "nx": (-uno, -tc, sc),
        "py": (sc, uno, tc),
        "ny": (sc, -uno, -tc),
        "pz": (sc, -tc, uno),
        "nz": (-sc, -tc, -uno),
    }[cara]
    v = np.stack(d, -1)
    return v / np.linalg.norm(v, axis=-1, keepdims=True)


CARAS = ["px", "nx", "py", "ny", "pz", "nz"]


def estrellas(cara, n, rng_semilla, cantidad, tinte=(1.0, 1.0, 1.0)):
    """Estrellas puntuales: direcciones al azar; las que caen en esta cara se proyectan a su píxel y se dibujan
    como una mancha gaussiana chica. Se usan las mismas estrellas en las seis caras (misma semilla)."""
    rng = np.random.default_rng(rng_semilla)
    pos = rng.normal(size=(cantidad, 3))
    pos /= np.linalg.norm(pos, axis=1, keepdims=True)
    brillo = rng.random(cantidad) ** 6 * 1.6 + 0.15
    color = np.array(tinte) * 0.7 + 0.3 * rng.random((cantidad, 3))
    x, y, z = pos[:, 0], pos[:, 1], pos[:, 2]
    ax, ay, az = np.abs(x), np.abs(y), np.abs(z)
    eje, signo = {"px": (0, 1), "nx": (0, -1), "py": (1, 1), "ny": (1, -1), "pz": (2, 1), "nz": (2, -1)}[cara]
    m = np.stack([ax, ay, az], 1)
    sel = (np.argmax(m, 1) == eje) & (np.sign(pos[:, eje]) == signo)
    x, y, z, ma = x[sel], y[sel], z[sel], m[sel, eje]
    sc, tc = {
        "px": (-z / ma, -y / ma), "nx": (z / ma, -y / ma),
        "py": (x / ma, z / ma), "ny": (x / ma, -z / ma),
        "pz": (x / ma, -y / ma), "nz": (-x / ma, -y / ma),
    }[cara]
    col = (sc + 1) / 2 * n - 0.5
    fila = (tc + 1) / 2 * n - 0.5
    b, c = brillo[sel], color[sel]
    luz = np.zeros((n, n, 3))
    sigma = 0.55 + 0.6 * np.clip(b, 0, 1)
    for dy in range(-2, 3):
        for dx in range(-2, 3):
            fi = np.round(fila).astype(int) + dy
            co = np.round(col).astype(int) + dx
            dist2 = (fi - fila) ** 2 + (co - col) ** 2
            peso = np.exp(-dist2 / (2 * sigma ** 2)) * b
            ok = (fi >= 0) & (fi < n) & (co >= 0) & (co < n)
            np.add.at(luz, (fi[ok], co[ok]), peso[ok, None] * c[ok])
    return luz


def skybox(nombre, n, color_fondo, colores_nube, semilla, sol=None):
    rng = np.random.default_rng(semilla)
    for cara in CARAS:
        d = direcciones_cara(cara, n)
        nube1 = fbm3(d * 1.6 + semilla, 6)
        nube2 = fbm3(d * 3.5 + 40 + semilla, 5)
        densidad = suave(-0.2, 0.5, nube1) * (0.6 + 0.4 * suave(-0.3, 0.6, nube2))
        filamentos = suave(0.25, 0.7, 1 - np.abs(nube2 * 2.2))
        img = np.broadcast_to(np.array(color_fondo), d.shape).copy()
        img += densidad[..., None] * np.array(colores_nube[0])
        img += (densidad * filamentos)[..., None] * np.array(colores_nube[1])
        img += (suave(0.45, 0.8, nube1) * suave(0.0, 0.5, nube2))[..., None] * np.array(colores_nube[2])
        img += estrellas(cara, n, semilla + 99, 9000)
        if sol is not None:
            centro = np.array(sol["dir"], float)
            centro /= np.linalg.norm(centro)
            ang = np.arccos(np.clip(d @ centro, -1, 1))
            disco = suave(sol["radio"] * 1.05, sol["radio"] * 0.95, ang)
            corona = np.exp(-((ang / sol["radio"]) - 1).clip(0) * 1.6) * (1 - disco)
            img = img * (1 - disco[..., None]) + disco[..., None] * np.array(sol["color"])
            img += corona[..., None] * np.array(sol["corona"])
        guardar("Skybox/%s_%s.png" % (nombre, cara), img)


# ---------------------------------------------------------------- texturas de superficie
def roca_volcanica(n=1024):
    rng = np.random.default_rng(1)
    base = fbm_periodico(n, 8, 6, rng)
    detalle = fbm_periodico(n, 64, 3, rng)
    f1, f2 = voronoi_periodico(n, 8, rng)
    grieta = suave(0.012, 0.0, f2 - f1)                      # bordes entre celdas
    brasa = grieta * suave(-0.2, 0.4, fbm_periodico(n, 6, 3, rng))
    gris = 0.13 + 0.07 * base + 0.04 * detalle
    img = np.stack([gris * 1.05, gris * 0.92, gris * 0.88], -1)
    img = mezcla(img, np.array([1.0, 0.32, 0.05]), brasa * 0.9)
    img = mezcla(img, np.array([0.05, 0.04, 0.04]), grieta * (1 - brasa) * 0.8)
    guardar("roca_volcanica.png", img)


def lava(n=512):
    rng = np.random.default_rng(2)
    flujo = fbm_periodico(n, 4, 6, rng)
    f1, f2 = voronoi_periodico(n, 5, rng)
    costra = suave(0.03, 0.09, f2 - f1) * suave(0.1, -0.25, flujo)   # placas oscuras que flotan
    calor = suave(-0.4, 0.6, flujo)
    img = mezcla(np.array([0.85, 0.12, 0.02]), np.array([1.0, 0.55, 0.05]), calor)
    img = mezcla(img, np.array([1.0, 0.92, 0.45]), suave(0.35, 0.75, flujo) * 0.8)
    img = mezcla(img, np.array([0.18, 0.05, 0.03]), costra * 0.85)
    guardar("lava.png", img)


def obsidiana(n=512):
    rng = np.random.default_rng(3)
    vetas = np.sin(np.linspace(0, 8 * np.pi, n)[None, :] + 6 * fbm_periodico(n, 4, 4, rng))
    g = 0.06 + 0.04 * vetas + 0.03 * fbm_periodico(n, 32, 3, rng)
    img = np.stack([g * 1.1, g * 0.8, g * 1.4], -1)
    guardar("obsidiana.png", img)


def piel_avatar(n=512):
    rng = np.random.default_rng(4)
    f1, f2 = voronoi_periodico(n, 6, rng)
    junta = suave(0.02, 0.0, f2 - f1)
    placa = 0.18 + 0.1 * fbm_periodico(n, 16, 4, rng) - 0.6 * f1
    img = np.stack([placa * 1.0, placa * 0.85, placa * 0.8], -1)
    img = mezcla(img, np.array([1.0, 0.45, 0.08]), junta)
    guardar("piel_avatar.png", img)


def caparazon(n=512):
    rng = np.random.default_rng(5)
    base = fbm_periodico(n, 8, 5, rng)
    f1, _ = voronoi_periodico(n, 4, rng)
    mancha = suave(0.08, 0.04, f1)
    img = mezcla(np.array([0.35, 0.04, 0.05]), np.array([0.55, 0.1, 0.06]), suave(-0.4, 0.4, base))
    img = mezcla(img, np.array([1.0, 0.75, 0.15]), mancha)
    guardar("caparazon_escarabajo.png", img)


def coloso(n=512):
    rng = np.random.default_rng(6)
    base = fbm_periodico(n, 6, 6, rng)
    f1, f2 = voronoi_periodico(n, 5, rng)
    veta = suave(0.015, 0.0, f2 - f1) * suave(0.0, 0.3, fbm_periodico(n, 4, 3, rng))
    g = 0.28 + 0.12 * base
    img = np.stack([g * 0.95, g * 0.88, g * 0.8], -1)
    img = mezcla(img, np.array([1.0, 0.5, 0.1]), veta)
    guardar("coloso.png", img)


def larva(n=256):
    rng = np.random.default_rng(8)
    b = fbm_periodico(n, 8, 4, rng)
    img = mezcla(np.array([1.0, 0.45, 0.05]), np.array([1.0, 0.85, 0.35]), suave(-0.3, 0.5, b))
    guardar("larva.png", img)


def casco_verde(n=1024):
    rng = np.random.default_rng(9)
    y, x = np.mgrid[0:n, 0:n] / n
    base = 0.85 + 0.15 * fbm_periodico(n, 16, 4, rng)
    verde = np.array([0.12, 0.6, 0.22])
    img = verde * base[..., None]
    # paneles: filas de alto fijo con cortes horizontales desfasados por fila
    filas = 12
    fila = np.floor(y * filas)
    desfase = (fila * 0.37) % 1
    linea_h = np.abs(((y * filas) % 1) - 0.5) > 0.47
    linea_v = np.abs((((x * 8 + desfase) % 1)) - 0.5) > 0.485
    img = mezcla(img, np.array([0.04, 0.2, 0.08]), (linea_h | linea_v).astype(float) * 0.8)
    # franja negra y franja clara a lo largo del casco (v es el eje largo en la esfera escalada)
    franja = (np.abs(y - 0.5) < 0.06).astype(float)
    borde = (np.abs(np.abs(y - 0.5) - 0.075) < 0.012).astype(float)
    img = mezcla(img, np.array([0.04, 0.05, 0.05]), franja * 0.95)
    img = mezcla(img, np.array([0.85, 0.95, 0.85]), borde)
    # remaches
    rem = ((((x * 64) % 1) - 0.5) ** 2 + ((((y * filas) % 1) - 0.1) ** 2) * 40) < 0.02
    img = mezcla(img, np.array([0.7, 0.8, 0.7]), rem.astype(float) * 0.5)
    guardar("casco_verde.png", img)


def cabina(n=256):
    y, x = np.mgrid[0:n, 0:n] / n
    brillo = np.exp(-((x - 0.35) ** 2 + (y - 0.7) ** 2) * 18)
    img = mezcla(np.array([0.02, 0.12, 0.08]), np.array([0.6, 1.0, 0.8]), brillo * 0.7)
    guardar("cabina.png", img)


def metal(n=512):
    rng = np.random.default_rng(10)
    y, x = np.mgrid[0:n, 0:n] / n
    cepillado = fbm_periodico(n, 4, 3, rng) * 0.4 + ruido_periodico(n, 256, rng) * 0.15
    g = 0.42 + 0.08 * cepillado
    panel = ((np.abs((x * 4) % 1 - 0.5) > 0.48) | (np.abs((y * 4) % 1 - 0.5) > 0.48)).astype(float)
    img = np.stack([g, g * 1.02, g * 1.06], -1)
    img = mezcla(img, np.array([0.15, 0.15, 0.17]), panel * 0.8)
    guardar("metal.png", img)


def plataforma(n=1024):
    rng = np.random.default_rng(12)
    y, x = np.mgrid[0:n, 0:n] / n
    cx, cy = x - 0.5, y - 0.5
    r = np.sqrt(cx * cx + cy * cy)
    ang = np.arctan2(cy, cx)
    g = 0.25 + 0.05 * fbm_periodico(n, 16, 4, rng)
    img = np.stack([g, g, g * 1.05], -1)
    # aro exterior con franjas amarillas y negras de advertencia
    aro = (r > 0.40) & (r < 0.47)
    franjas = (((ang / (2 * np.pi)) * 36) % 1) < 0.5
    img = mezcla(img, np.array([0.95, 0.75, 0.05]), (aro & franjas).astype(float))
    img = mezcla(img, np.array([0.05, 0.05, 0.05]), (aro & ~franjas).astype(float))
    # círculos y flechas hacia el centro (marca de aterrizaje original)
    anillo = (np.abs(r - 0.3) < 0.012) | (np.abs(r - 0.12) < 0.015)
    img = mezcla(img, np.array([0.9, 0.9, 0.9]), anillo.astype(float))
    for k in range(3):
        a0 = k * 2 * np.pi / 3
        d = np.angle(np.exp(1j * (ang - a0)))
        flecha = (np.abs(d) < (0.28 - r) * 1.6) & (r > 0.15) & (r < 0.27)
        img = mezcla(img, np.array([1.0, 0.45, 0.05]), flecha.astype(float))
    guardar("plataforma.png", img)


def runas(n=512):
    rng = np.random.default_rng(13)
    y, x = np.mgrid[0:n, 0:n] / n
    cx, cy = x - 0.5, y - 0.5
    r = np.sqrt(cx * cx + cy * cy)
    ang = np.arctan2(cy, cx)
    g = 0.12 + 0.05 * fbm_periodico(n, 8, 4, rng)
    img = np.stack([g, g * 0.9, g * 0.95], -1)
    brilla = (np.abs(r - 0.44) < 0.008) | (np.abs(r - 0.36) < 0.006) | (np.abs(r - 0.16) < 0.006)
    # glifos: marcas radiales y triángulos entre los dos aros (diseño original)
    sector = ((ang / (2 * np.pi)) * 16) % 1
    marca = (r > 0.37) & (r < 0.43) & (np.abs(sector - 0.5) < 0.06 + 0.25 * (r - 0.37) / 0.06 * (sector > 0.5))
    tri = (r < 0.34) & (r > 0.18) & (np.abs(np.angle(np.exp(1j * ang * 3))) < (0.34 - r) * 3.5)
    brilla = brilla | marca | tri
    img = mezcla(img, np.array([1.0, 0.55, 0.12]), brilla.astype(float))
    guardar("runas.png", img)


def humo(n=256):
    rng = np.random.default_rng(14)
    y, x = np.mgrid[0:n, 0:n] / n
    r = np.sqrt((x - 0.5) ** 2 + (y - 0.5) ** 2) * 2
    alfa = suave(1.0, 0.2, r) * (0.75 + 0.25 * fbm_periodico(n, 6, 4, rng))
    guardar("humo.png", np.ones((n, n, 3)), alfa)


def asteroide(n=512):
    rng = np.random.default_rng(15)
    b = fbm_periodico(n, 8, 6, rng)
    f1, _ = voronoi_periodico(n, 4, rng)
    crater = suave(0.06, 0.02, f1) * 0.25
    g = 0.32 + 0.12 * b - crater
    img = np.stack([g * 1.05, g * 0.95, g * 0.85], -1)
    guardar("asteroide.png", img)


def panel_solar(n=256):
    y, x = np.mgrid[0:n, 0:n] / n
    celda = ((np.abs((x * 6) % 1 - 0.5) > 0.44) | (np.abs((y * 3) % 1 - 0.5) > 0.44)).astype(float)
    img = mezcla(np.array([0.08, 0.15, 0.45]), np.array([0.8, 0.8, 0.85]), celda)
    guardar("panel_solar.png", img)


def generico(n=512):
    rng = np.random.default_rng(16)
    b = fbm_periodico(n, 8, 6, rng)
    g = 0.7 + 0.2 * b
    guardar("planeta_generico.png", np.stack([g, g, g], -1))
    b2 = fbm_periodico(n, 16, 5, rng)
    g2 = 0.6 + 0.15 * b2
    guardar("piso_generico.png", np.stack([g2, g2, g2], -1))


def cristal(n=256):
    y, x = np.mgrid[0:n, 0:n] / n
    g = 0.6 + 0.4 * np.abs(np.sin(y * 9 + x * 3))
    img = np.stack([g, g * 0.75, g * 0.35], -1)
    guardar("cristal.png", img)


# ---------------------------------------------------------------- planeta Pyros y su luna (equirectangulares)
def pyros_planeta(ancho=2048, alto=1024):
    d = direcciones_equirect(ancho, alto)
    elev = fbm3(d * 2.2, 6)
    grietas = 1 - np.abs(fbm3(d * 5.0 + 9, 5)) * 3.0
    remolino = fbm3(d * 6.0 + 20, 4)
    mar = suave(0.02, -0.12, elev)                               # 1 donde hay mares de lava
    costra = np.stack([0.16 + 0.06 * elev, 0.09 + 0.03 * elev, 0.07 + 0.02 * elev], -1)
    lava_c = mezcla(np.array([0.9, 0.18, 0.02]), np.array([1.0, 0.75, 0.15]), suave(-0.3, 0.5, remolino))
    img = mezcla(costra, lava_c, mar)
    img = mezcla(img, np.array([1.0, 0.4, 0.05]), suave(0.55, 0.95, grietas) * (1 - mar) * 0.9)
    guardar("pyros_planeta.png", img)


def luna_ceniza(ancho=1024, alto=512):
    d = direcciones_equirect(ancho, alto)
    b = fbm3(d * 3.0 + 50, 6)
    rng = np.random.default_rng(17)
    g = 0.45 + 0.12 * b
    centros = rng.normal(size=(40, 3))
    centros /= np.linalg.norm(centros, axis=1, keepdims=True)
    radios = rng.random(40) * 0.18 + 0.04
    for c, rad in zip(centros, radios):
        ang = np.arccos(np.clip(d @ c, -1, 1)) / rad
        g = g - 0.12 * suave(1.0, 0.7, ang) + 0.08 * suave(0.85, 1.0, ang) * suave(1.15, 1.0, ang)
    guardar("luna_ceniza.png", np.stack([g, g * 0.97, g * 0.95], -1))


if __name__ == "__main__":
    roca_volcanica()
    lava()
    obsidiana()
    piel_avatar()
    caparazon()
    coloso()
    larva()
    casco_verde()
    cabina()
    metal()
    plataforma()
    runas()
    humo()
    asteroide()
    panel_solar()
    generico()
    cristal()
    pyros_planeta()
    luna_ceniza()
    skybox("espacio", 1024, (0.004, 0.01, 0.035),
           [(0.05, 0.16, 0.45), (0.15, 0.35, 0.75), (0.5, 0.6, 0.85)], 3)
    skybox("pyros", 1024, (0.025, 0.004, 0.008),
           [(0.42, 0.08, 0.04), (0.65, 0.22, 0.06), (0.9, 0.55, 0.25)], 5,
           sol={"dir": (0.35, 0.3, -0.88), "radio": 0.15, "color": (1.0, 0.85, 0.55), "corona": (0.9, 0.3, 0.08)})
