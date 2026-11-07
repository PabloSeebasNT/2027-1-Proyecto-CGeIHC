#include "OooPaisaje.h"
#include "PlanetaOoo.h"
#include "OooCasaArbol.h"
#include "OooJardin.h"
#include "PersonajesOoo.h"
#include "GrumosaOoo.h"
#include "MentaOoo.h"
#include "Geometria.h"
#include "OooModelosNature.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

namespace
{
    const float PI = 3.14159265358979323846f;
    const float LIMITE_VISUAL = 108.0f;
    Texture* pasto = nullptr;
    Texture* tierra = nullptr;
    Texture* madera = nullptr;
    Texture* hojas = nullptr;
    Texture* piedra = nullptr;
    MeshModel* relieve = nullptr;
    MeshModel* senderos = nullptr;
    MeshModel* claro = nullptr;
    std::vector<glm::vec2> ejesSenderos;

    struct Arbol { float x, z, tamano; int forma; };
    const Arbol arboles[] = {
        {-88,  12, 1.15f, 0}, {-90,  43, 1.05f, 1}, {-76,  65, 1.25f, 0},
        {-53,  82, 1.10f, 0}, {-21,  91, 1.32f, 1}, { 18,  87, 1.08f, 0},
        { 56,  76, 1.18f, 0}, { 84,  47, 1.35f, 1}, { 94,  13, 1.12f, 0},
        { 88, -27, 1.28f, 0}, { 75, -69, 1.05f, 1}, { 49, -90, 1.13f, 0},
        { 15, -93, 1.22f, 1}, {-26, -92, 1.16f, 1}, {-67, -73, 1.27f, 0},
        {-88, -42, 1.06f, 1}, {-60,  17, 0.86f, 0}, {-43,  43, 0.95f, 0},
        { 47,  36, 0.93f, 1}, { 63, -11, 0.95f, 0}, {-11, -68, 1.03f, 0},
        { 25, -68, 0.97f, 1},
        // Grupos del bosque: claros entre ellos para las rutas y las vistas.
        {-95,-12,.86f,0},{-94,27,.78f,1},{-84,55,.92f,0},{-66,77,.85f,0},
        {-39,87,1.04f,1},{-6,94,.82f,0},{32,88,.96f,0},{69,68,.84f,1},
        {86,32,.78f,0},{94,-3,.91f,1},{91,-44,.83f,0},{63,-79,.94f,0},
        {34,-90,.81f,1},{-8,-88,1.02f,0},{-43,-84,.85f,0},{-77,-59,.76f,1},
        {75,12,.82f,0},{72,42,1.03f,1},{60,53,.76f,0},{-57,58,.83f,0}
    };

    struct Roca { float x, z, radio; };
    const Roca piedrasGrandes[] = {
        {-72, 33, 2.1f}, {-69,-48,2.7f}, {73,27,2.0f}, {78,-43,2.3f},
        {-17,77,1.7f}, {14,-80,2.1f}, {65,64,1.8f},
        {-88,29,1.3f},{-65,70,1.1f},{-33,80,1.6f},{30,80,1.2f},
        {76,52,1.5f},{82,-18,1.2f},{59,-73,1.6f},{-43,-77,1.1f},{57,14,1.3f}
    };

    float Suavizar(float v)
    {
        v = glm::clamp(v, 0.0f, 1.0f);
        return v * v * (3.0f - 2.0f * v);
    }

    float AleatorioDeterminista(float semilla)
    {
        float s = std::sin(semilla * 12.9898f + 78.233f) * 43758.5453f;
        return s - std::floor(s);
    }

    glm::mat4 Colocar(float x, float y, float z, glm::vec3 escala)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), CENTRO_OOO + glm::vec3(x, y, z));
        return glm::scale(m, escala);
    }

    glm::mat4 ColocarGirado(float x, float y, float z, glm::vec3 escala, float rotY, float rotZ = 0.0f)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), CENTRO_OOO + glm::vec3(x, y, z));
        if (rotY != 0.0f) m = glm::rotate(m, glm::radians(rotY), glm::vec3(0, 1, 0));
        if (rotZ != 0.0f) m = glm::rotate(m, glm::radians(rotZ), glm::vec3(0, 0, 1));
        return glm::scale(m, escala);
    }

    float AlturaTerreno(float x, float z)
    {
        float radio = std::sqrt(x * x + z * z);
        float borde = Suavizar((radio - 111.0f) / 68.0f);
        float ondulacion = std::sin(x * 0.037f + 0.4f) * std::cos(z * 0.046f);
        ondulacion += 0.45f * std::sin(z * 0.082f - x * 0.027f);
        return 0.06f + borde * (11.0f + 5.5f * ondulacion);
    }

    MeshModel* CrearRelieve()
    {
        const int N = 100;
        const float EXTENSION = 200.0f;
        std::vector<GLfloat> vertices;
        std::vector<unsigned int> indices;
        vertices.reserve((N + 1) * (N + 1) * 8);
        indices.reserve(N * N * 6);
        for (int z = 0; z <= N; ++z)
        {
            for (int x = 0; x <= N; ++x)
            {
                float px = -EXTENSION + 2.0f * EXTENSION * x / N;
                float pz = -EXTENSION + 2.0f * EXTENSION * z / N;
                float h = AlturaTerreno(px, pz);
                float dx = (AlturaTerreno(px + 0.3f, pz) - AlturaTerreno(px - 0.3f, pz)) / 0.6f;
                float dz = (AlturaTerreno(px, pz + 0.3f) - AlturaTerreno(px, pz - 0.3f)) / 0.6f;
                glm::vec3 normal = glm::normalize(glm::vec3(-dx, 1.0f, -dz));
                AgregarVertice(vertices, glm::vec3(px, h, pz), px * 0.11f, pz * 0.11f, normal);
            }
        }
        const unsigned int paso = N + 1;
        for (int z = 0; z < N; ++z)
            for (int x = 0; x < N; ++x)
            {
                unsigned int a = z * paso + x;
                indices.insert(indices.end(), { a, a + paso, a + 1,
                    a + 1, a + paso, a + paso + 1 });
            }
        return GuardarMalla(vertices, indices);
    }

    glm::vec2 Curva(const glm::vec2& a, const glm::vec2& b,
        const glm::vec2& c, const glm::vec2& d, float t)
    {
        return 0.5f * ((2.0f * b) + (-a + c) * t +
            (2.0f * a - 5.0f * b + 4.0f * c - d) * (t * t) +
            (-a + 3.0f * b - 3.0f * c + d) * (t * t * t));
    }

    void AgregarCamino(const std::vector<glm::vec2>& puntos,
        std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices, float ancho)
    {
        std::vector<glm::vec2> muestras;
        for (size_t k = 0; k + 1 < puntos.size(); ++k)
            for (int j = 0; j < 12; ++j)
            {
                const glm::vec2& a = puntos[k == 0 ? 0 : k - 1];
                const glm::vec2& b = puntos[k];
                const glm::vec2& c = puntos[k + 1];
                const glm::vec2& d = puntos[std::min(k + 2, puntos.size() - 1)];
                muestras.push_back(Curva(a, b, c, d, j / 12.0f));
            }
        muestras.push_back(puntos.back());
        ejesSenderos.insert(ejesSenderos.end(), muestras.begin(), muestras.end());
        if (muestras.size() < 2) return;

        unsigned int base = static_cast<unsigned int>(vertices.size() / 8);
        float distancia = 0.0f;
        for (size_t i = 0; i < muestras.size(); ++i)
        {
            glm::vec2 anterior = muestras[i == 0 ? 0 : i - 1];
            glm::vec2 siguiente = muestras[std::min(i + 1, muestras.size() - 1)];
            glm::vec2 direccion = siguiente - anterior;
            if (glm::dot(direccion, direccion) < 0.0001f) direccion = glm::vec2(0.0f, 1.0f);
            direccion = glm::normalize(direccion);
            glm::vec2 lateral(-direccion.y, direccion.x);
            if (i > 0) distancia += glm::length(muestras[i] - muestras[i - 1]);
            float ondulacion = 1.0f + 0.045f * std::sin(distancia * 0.35f);
            glm::vec2 lado = lateral * 0.5f * ancho * ondulacion;
            glm::vec2 izquierda = muestras[i] - lado;
            glm::vec2 derecha = muestras[i] + lado;
            AgregarVertice(vertices, glm::vec3(izquierda.x, 0.15f, izquierda.y),
                0.0f, distancia * 0.28f, glm::vec3(0, 1, 0));
            AgregarVertice(vertices, glm::vec3(derecha.x, 0.15f, derecha.y),
                1.0f, distancia * 0.28f, glm::vec3(0, 1, 0));
            if (i > 0)
            {
                unsigned int a = base + static_cast<unsigned int>(2 * (i - 1));
                indices.insert(indices.end(), { a, a + 1, a + 2, a + 1, a + 3, a + 2 });
            }
        }
    }

    MeshModel* CrearSenderos()
    {
        std::vector<GLfloat> v;
        std::vector<unsigned int> idx;
        glm::vec2 entrada = EntradaCasaArbolOoo();
        AgregarCamino({ {13,70},{9,48},{4,26},{-14,20},entrada }, v, idx, 3.8f);
        AgregarCamino({ {4,26},{19,20},{28,1},{43,-33} }, v, idx, 3.0f);
        return GuardarMalla(v, idx);
    }

    MeshModel* CrearDisco()
    {
        std::vector<GLfloat> v;
        std::vector<unsigned int> idx;
        AgregarVertice(v, glm::vec3(0, 0, 0), .5f, .5f, glm::vec3(0, 1, 0));
        for (int k = 0; k <= 40; ++k)
        {
            float a = 2 * PI * k / 40;
            float x = std::cos(a), z = std::sin(a);
            AgregarVertice(v, glm::vec3(x, 0, z), .5f + .5f * x, .5f + .5f * z,
                glm::vec3(0, 1, 0));
            if (k > 0) idx.insert(idx.end(), { 0, static_cast<unsigned int>(k+1), static_cast<unsigned int>(k) });
        }
        return GuardarMalla(v, idx);
    }

    void DibujarRama(float x, float y, float z, float tamano, float angulo, float largo)
    {
        glm::mat4 rama = glm::translate(glm::mat4(1.0f), CENTRO_OOO + glm::vec3(x, y, z));
        rama = glm::rotate(rama, glm::radians(angulo), glm::vec3(0, 0, 1));
        rama = glm::rotate(rama, glm::radians(angulo * 0.18f), glm::vec3(0, 1, 0));
        Dibujar(mCilindro, glm::scale(rama, glm::vec3(.28f * tamano, largo * tamano, .28f * tamano)),
            glm::vec3(.92f, .83f, .71f));
    }

    void DibujarArbol(const Arbol& a)
    {
        float t = a.tamano;
        float altura = (a.forma == 0 ? 7.4f : 8.8f) * t;
        UsarMaterial(matOpaco);
        madera->UseTexture();
        Dibujar(mCilindro, Colocar(a.x, altura * 0.48f, a.z,
            glm::vec3(0.85f * t, altura * .96f, 0.85f * t)), glm::vec3(.94f, .83f, .71f));

        DibujarRama(a.x - .35f * t, altura * .70f, a.z + .15f * t, t, -34.0f, 2.5f);
        DibujarRama(a.x + .42f * t, altura * .77f, a.z - .06f * t, t, 38.0f, 2.2f);
        if (a.forma == 0)
            DibujarRama(a.x + .08f * t, altura * .60f, a.z + .40f * t, t * .86f, 16.0f, 1.8f);

        hojas->UseTexture();
        if (a.forma == 0)
        {
            struct Copa { float dx, dy, dz, sx, sy, sz, tono; };
            const Copa copas[] = {
                { 0.0f,  1.05f,  0.0f, 3.25f, 2.55f, 3.00f, .00f },
                {-1.6f,  0.18f,  0.8f, 2.05f, 1.90f, 1.95f, .10f },
                { 1.7f,  0.38f, -0.7f, 2.18f, 1.92f, 2.00f, .06f },
                { 0.6f,  1.60f,  1.2f, 1.65f, 1.55f, 1.52f, .14f },
                {-0.8f,  1.32f, -1.1f, 1.52f, 1.48f, 1.52f, .08f },
                { 0.0f,  2.18f,  0.0f, 1.18f, 1.04f, 1.12f, .18f }
            };
            for (const Copa& c : copas)
            {
                glm::vec3 color = glm::vec3(.73f + .10f * c.tono, .95f + .03f * c.tono, .73f + .06f * c.tono);
                Dibujar(mEsferaBaja,
                    Colocar(a.x + c.dx * t, altura + c.dy * t, a.z + c.dz * t,
                        glm::vec3(c.sx * t, c.sy * t, c.sz * t)), color);
            }
        }
        else
        {
            struct Nivel { float dy, sx, sy, sz, dx, dz, tono; };
            const Nivel niveles[] = {
                { .52f, 3.10f, 1.52f, 3.10f,  0.0f,  0.0f, .00f },
                { 1.78f, 2.62f, 1.40f, 2.62f, -0.08f, 0.10f, .07f },
                { 2.86f, 2.05f, 1.22f, 2.10f,  0.12f,-0.07f, .12f },
                { 3.78f, 1.58f, 1.08f, 1.64f,  0.0f,  0.0f, .18f }
            };
            for (const Nivel& n : niveles)
            {
                glm::vec3 color = glm::vec3(.72f + .08f * n.tono, .93f + .04f * n.tono, .74f + .07f * n.tono);
                Dibujar(mEsferaBaja,
                    Colocar(a.x + n.dx * t, altura * .42f + n.dy * t, a.z + n.dz * t,
                        glm::vec3(n.sx * t, n.sy * t, n.sz * t)), color);
            }
            Dibujar(mEsferaBaja, Colocar(a.x + .32f * t, altura * .70f, a.z - .55f * t,
                glm::vec3(1.18f * t, .82f * t, 1.08f * t)), glm::vec3(.77f, .96f, .78f));
        }
    }

    void DibujarArbustos()
    {
        hojas->UseTexture();
        const glm::vec2 centros[] = {
            {-51,27},{-62,50},{-45,67},{-11,72},{28,65},{65,42},
            {75,-11},{70,-57},{31,-72},{-37,-66},{-72,-25},{-48,-9},
            {51,8},{-9,-42},{-1,50},{42,-55}
        };
        for (int i = 0; i < 16; ++i)
        {
            const glm::vec2 c = centros[i];
            float r = 1.0f + 0.22f * (i % 4);
            float sesgo = AleatorioDeterminista(i * 1.71f) * 2.0f - 1.0f;
            struct Bulto { float dx, dy, dz, sx, sy, sz, tono; };
            const Bulto bultos[] = {
                { 0.0f, .54f, 0.0f, 1.60f, .74f, 1.22f, .00f },
                { .68f, .48f, .20f, 1.02f, .58f, .92f, .08f },
                {-.58f, .42f,-.12f, .92f, .52f, .88f, .04f },
                { .12f, .74f,-.50f, .82f, .46f, .74f, .12f },
                {-.18f, .64f, .56f, .72f, .42f, .68f, .15f }
            };
            for (const Bulto& b : bultos)
            {
                glm::vec3 color = glm::vec3(.64f + .08f * b.tono, .91f + .04f * b.tono, .71f + .05f * b.tono);
                Dibujar(mEsferaBaja, Colocar(c.x + (b.dx + .14f * sesgo) * r, b.dy * r, c.y + b.dz * r,
                    glm::vec3(b.sx * r, b.sy * r, b.sz * r)), color);
            }
        }
    }

    void DibujarFlores()
    {
        const glm::vec2 parches[] = { {-21,20},{-50,-8},{-29,9},{7,-8},
            {34,1},{-4,57},{46,51},{24,-52},{-55,61} };
        for (int parche = 0; parche < 9; ++parche)
            for (int f = 0; f < 5; ++f)
            {
                float a = 2.4f * f + .7f * parche;
                float r = 1.8f + (f % 3) * 1.2f;
                float x = parches[parche].x + r * std::cos(a);
                float z = parches[parche].y + r * std::sin(a);
                // Mantener libres el acceso, el agua y la huella de la casa.
                glm::vec2 punto(x,z);
                bool sobreCamino = false;
                for (const glm::vec2& eje : ejesSenderos)
                    if (glm::length(punto-eje)<3.2f) { sobreCamino=true; break; }
                if (sobreCamino || glm::length(EvitarLagoOoo(punto,.7f)-punto)>.01f ||
                    glm::length(EvitarCasaArbolOoo(punto,1.0f)-punto)>.01f) continue;
                float h = .58f + .15f * (f % 3);
                madera->UseTexture();
                Dibujar(mCilindro, Colocar(x, .06f + h*.5f, z,
                    glm::vec3(.06f, h, .06f)), glm::vec3(.56f,.91f,.54f));
                piedra->UseTexture();
                const glm::vec3 petalo = (parche + f) % 3 == 0 ? glm::vec3(1.0f,.55f,.75f)
                    : ((parche + f) % 3 == 1 ? glm::vec3(.85f,.76f,1.0f) : glm::vec3(1.0f,.9f,.53f));
                for (int k = 0; k < 5; ++k)
                {
                    float ag = 2.0f * PI * k / 5.0f;
                    Dibujar(mEsferaBaja,
                        Colocar(x + .22f*std::cos(ag), .10f+h, z + .22f*std::sin(ag),
                        glm::vec3(.23f, .12f, .23f)), petalo);
                }
                Dibujar(mEsferaBaja, Colocar(x,.12f+h,z, glm::vec3(.18f,.16f,.18f)),
                    glm::vec3(1.0f,.80f,.20f));
            }
    }

    void DibujarRocaCompuesta(float x, float z, float radio, float semilla)
    {
        piedra->UseTexture();
        glm::vec3 baseColor(.80f, .86f, .79f);
        float giro = 15.0f + 35.0f * AleatorioDeterminista(semilla + .2f);
        struct Trozo { float dx, dy, dz, sx, sy, sz, tono, rotZ; };
        const Trozo trozos[] = {
            { 0.0f, .58f, 0.0f, 1.00f, .62f, .84f, .00f,  0.0f },
            {-.44f, .42f, .18f, .58f, .38f, .48f, -.05f, 12.0f },
            { .46f, .47f,-.14f, .52f, .34f, .56f,  .07f,-10.0f },
            { .10f, .70f, .28f, .42f, .24f, .34f,  .13f, 18.0f }
        };
        for (const Trozo& t : trozos)
        {
            glm::vec3 color = baseColor + glm::vec3(t.tono * .4f, t.tono * .35f, t.tono * .3f);
            Dibujar(mEsferaBaja,
                ColocarGirado(x + t.dx * radio, t.dy * radio, z + t.dz * radio,
                    glm::vec3(t.sx * radio, t.sy * radio, t.sz * radio), giro, t.rotZ),
                color);
        }
    }

    void DibujarPiedras()
    {
        for (int i = 0; i < 7; ++i)
            DibujarRocaCompuesta(piedrasGrandes[i].x, piedrasGrandes[i].z, piedrasGrandes[i].radio, 13.0f + i * 2.3f);

        for (int i = 0; i < 26; ++i)
        {
            float a = 2.0f*PI*(i + .32f) / 26;
            float r = 105.0f + 9.0f * std::sin(i * 2.17f);
            float x = r*std::cos(a), z = r*std::sin(a);
            float s = .55f + .27f*(i%4);
            DibujarRocaCompuesta(x, z, s, 71.0f + i * 1.2f);
        }
    }

    glm::vec2 SepararDe(glm::vec2 punto, glm::vec2 centro, float radio)
    {
        glm::vec2 d = punto-centro;
        float distancia2 = glm::dot(d, d);
        if (distancia2 >= radio * radio) return punto;
        if (distancia2 < 0.000001f) return centro + glm::vec2(radio, 0.0f);
        return centro + d * (radio / std::sqrt(distancia2));
    }
}

void CargarPaisajeOoo()
{
    pasto = CargarTextura("Textures/OooEtapa1/cesped.png");
    tierra = CargarTextura("Textures/OooEtapa1/sendero.png");
    madera = CargarTextura("Textures/OooEtapa1/corteza.png");
    hojas = CargarTextura("Textures/OooEtapa1/hojas.png");
    piedra = CargarTextura("Textures/OooEtapa1/piedra.png");
    relieve = CrearRelieve();
    senderos = CrearSenderos();
    claro = CrearDisco();
    CargarCasaArbolOoo();
    CargarJardinOoo();
    CargarModelosNatureOoo();
}

void DibujarPaisajeOoo()
{
    UsarMaterial(matOpaco);
    pasto->UseTexture();
    Dibujar(relieve, glm::translate(glm::mat4(1.0f), CENTRO_OOO), glm::vec3(.98f, 1.0f, .91f));

    tierra->UseTexture();
    glm::vec2 entrada = EntradaCasaArbolOoo();
    Dibujar(claro, Colocar(entrada.x,.125f,entrada.y,glm::vec3(4.0f,1.0f,3.1f)),
        glm::vec3(.98f,.94f,.85f));
    Dibujar(claro, Colocar(43.0f,.125f,-33.0f,glm::vec3(10.0f,1.0f,8.5f)),
        glm::vec3(.92f,1.0f,.87f));
    Dibujar(senderos, glm::translate(glm::mat4(1.0f), CENTRO_OOO), glm::vec3(1.0f,.94f,.86f));

    DibujarModelosNatureOoo();


    DibujarCasaArbolOoo();
    DibujarJardinOoo();
    DibujarFlores();
}

float AlturaPaseoOoo(glm::vec2 posicion)
{
    float distancia=1000.0f;
    for (const glm::vec2& p : ejesSenderos) distancia=std::min(distancia,glm::length(posicion-p));
    // Transicion corta al borde del camino para no dar un salto de nueve centimetros.
    float camino=1.0f-Suavizar((distancia-1.4f)/.5f);
    return std::max(AlturaTerreno(posicion.x,posicion.y),.06f+.09f*camino);
}

glm::vec2 CorregirPosicionLocalOoo(glm::vec2 posicion, float radioPersonaje, bool evitarAmigos)
{
    float maxRadio = std::max(1.0f, LIMITE_VISUAL - radioPersonaje);
    float distancia2 = glm::dot(posicion, posicion);
    if (distancia2 > maxRadio * maxRadio)
        posicion = glm::normalize(posicion) * maxRadio;

    for (int intento = 0; intento < 2; ++intento)
    {
        for (const Arbol& a : arboles)
            posicion = SepararDe(posicion, glm::vec2(a.x, a.z),
                (0.65f * a.tamano) + radioPersonaje + .20f);
        for (const Roca& r : piedrasGrandes)
            posicion = SepararDe(posicion, glm::vec2(r.x, r.z),
                .75f*r.radio + radioPersonaje);
        distancia2 = glm::dot(posicion,posicion);
        if (distancia2 > maxRadio * maxRadio)
            posicion = glm::normalize(posicion) * maxRadio;
    }
    posicion = EvitarCasaArbolOoo(posicion, radioPersonaje);
    posicion = EvitarLagoOoo(posicion, radioPersonaje);
    if (evitarAmigos) posicion = EvitarPersonajesOoo(posicion, radioPersonaje);
    if (evitarAmigos) posicion = EvitarGrumosaOoo(posicion, radioPersonaje);
    if (evitarAmigos) posicion = EvitarMentaOoo(posicion, radioPersonaje);
    return posicion;
}

glm::vec3 CorregirPosicionAvatarOoo(glm::vec3 posicion, float radioPersonaje)
{
    glm::vec2 local(posicion.x - CENTRO_OOO.x, posicion.z - CENTRO_OOO.z);
    local = CorregirPosicionLocalOoo(local, radioPersonaje);
    posicion.x = CENTRO_OOO.x + local.x;
    posicion.z = CENTRO_OOO.z + local.y;
    return posicion;
}
