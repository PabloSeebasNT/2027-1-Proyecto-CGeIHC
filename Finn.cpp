#include "Finn.h"
#include "Geometria.h"
#include "OooPaisaje.h"
#include <glfw3.h>
#include <gtc/matrix_transform.hpp>
#include <cmath>

namespace
{
    // Indices de las piezas tal como se exportaron desde Blender.
    enum ParteFinn
    {
        TORSO, CABEZA,
        BRAZO_DER, ANTEBRAZO_DER, BRAZO_IZQ, ANTEBRAZO_IZQ,
        MUSLO_DER, PIERNA_DER, MUSLO_IZQ, PIERNA_IZQ
    };

    const char* archivos[] = {
        "cuerpofinn", "cabezafinn",
        "brazosupder", "brazoinfder", "brazosupizq", "brazoinfizq",
        "piernasupder", "piernainfder", "piernasupizq", "piernainfizq"
    };
    Model piezas[10];
    // Finn se mueve aparte del avatar principal y solo cuando se pulsan las flechas.
    glm::vec3 desplazamientoFinn(0.0f);
    float rumboFinn = 0.0f;
    float faseCaminataFinn = 0.0f;
    float intensidadCaminataFinn = 0.0f;
    bool caminandoFinn = false;

    const float VELOCIDAD_FINN = 4.2f;           // unidades por segundo
    const float FASE_POR_UNIDAD = 1.75f;        // los pasos siguen la distancia recorrida
    bool saludoIniciado = false;
    float inicioSaludo = 0.0f;

    const float DURACION_SALUDO = 3.1f;
    const float SUBIR_BRAZO = 0.55f;
    const float BAJAR_BRAZO = 2.45f;

    float Suavizar(float t)
    {
        t = glm::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    float IntensidadSaludo(float t)
    {
        if (t < 0.0f || t >= DURACION_SALUDO) return 0.0f;
        if (t < SUBIR_BRAZO) return Suavizar(t / SUBIR_BRAZO);
        if (t > BAJAR_BRAZO)
            return 1.0f - Suavizar((t - BAJAR_BRAZO) / (DURACION_SALUDO - BAJAR_BRAZO));
        return 1.0f;
    }

    const float BASE_ORIGINAL = 0.596f;
    const float ALTURA_ORIGINAL = 73.348f - BASE_ORIGINAL;
    const float ALTURA_FINN = 3.0f;

    // Las coordenadas son de los OBJ, antes de reducir el modelo.
    // Se tomaron de los anillos donde se unen las piezas, no de sus centros.
    const glm::vec3 CUELLO(0.0f, 56.66f, -0.01f);
    const glm::vec3 HOMBRO_DER(-11.54f, 55.03f, 0.08f);
    const glm::vec3 CODO_DER(-15.98f, 42.86f, 0.08f);
    const glm::vec3 HOMBRO_IZQ(11.54f, 55.03f, 0.08f);
    const glm::vec3 CODO_IZQ(15.98f, 42.86f, 0.08f);
    const glm::vec3 CADERA_DER(-7.63f, 33.67f, 0.09f);
    const glm::vec3 RODILLA_DER(-8.63f, 18.56f, 0.13f);
    const glm::vec3 CADERA_IZQ(7.63f, 33.67f, 0.09f);
    const glm::vec3 RODILLA_IZQ(8.63f, 18.56f, 0.13f);

    void DibujarBrazo(const glm::mat4& torso, int superior, int inferior,
        const glm::vec3& puntoHombro, const glm::vec3& puntoCodo,
        float giroHombro, float giroCodo,
        float giroLateralHombro = 0.0f, float giroLateralCodo = 0.0f)
    {
        // El hombro nace del torso. El antebrazo nace del hombro, no del mundo.
        glm::mat4 hombro = glm::translate(torso, puntoHombro);
        hombro = glm::rotate(hombro, glm::radians(giroHombro), glm::vec3(1.0f, 0.0f, 0.0f));
        hombro = glm::rotate(hombro, glm::radians(giroLateralHombro), glm::vec3(0.0f, 0.0f, 1.0f));
        DibujarModelo(piezas[superior], glm::translate(hombro, -puntoHombro));

        glm::mat4 codo = glm::translate(hombro, puntoCodo - puntoHombro);
        codo = glm::rotate(codo, glm::radians(giroCodo), glm::vec3(1.0f, 0.0f, 0.0f));
        codo = glm::rotate(codo, glm::radians(giroLateralCodo), glm::vec3(0.0f, 0.0f, 1.0f));
        DibujarModelo(piezas[inferior], glm::translate(codo, -puntoCodo));
    }

    void DibujarPierna(const glm::mat4& torso, int superior, int inferior,
        const glm::vec3& puntoCadera, const glm::vec3& puntoRodilla,
        float giroCadera, float giroRodilla)
    {
        glm::mat4 cadera = glm::translate(torso, puntoCadera);
        cadera = glm::rotate(cadera, glm::radians(giroCadera), glm::vec3(1.0f, 0.0f, 0.0f));
        DibujarModelo(piezas[superior], glm::translate(cadera, -puntoCadera));

        glm::mat4 rodilla = glm::translate(cadera, puntoRodilla - puntoCadera);
        rodilla = glm::rotate(rodilla, glm::radians(giroRodilla), glm::vec3(1.0f, 0.0f, 0.0f));
        DibujarModelo(piezas[inferior], glm::translate(rodilla, -puntoRodilla));
    }
}

void CargarFinn()
{
    for (int i = 0; i < 10; ++i)
        piezas[i].LoadModel(std::string("Models/Finn/") + archivos[i] + ".obj");
}

void ActualizarFinn(const bool* teclas, float dt, float radioCaminable)
{
    if (dt <= 0.0f) return;

    // Las flechas quedan libres: WASD sigue manejando al avatar y la camara.
    glm::vec3 direccion(0.0f);
    if (teclas != nullptr)
    {
        if (teclas[GLFW_KEY_UP])    direccion.z -= 1.0f;
        if (teclas[GLFW_KEY_DOWN])  direccion.z += 1.0f;
        if (teclas[GLFW_KEY_LEFT])  direccion.x -= 1.0f;
        if (teclas[GLFW_KEY_RIGHT]) direccion.x += 1.0f;
    }

    caminandoFinn = false;
    if (glm::dot(direccion, direccion) > 0.0f)
    {
        direccion = glm::normalize(direccion);  // diagonal igual de rapida que recto
        glm::vec3 anterior = desplazamientoFinn;
        desplazamientoFinn += direccion * VELOCIDAD_FINN * dt;

        // El piso de Ooo tiene un radio caminable; Finn no debe salirse.
        glm::vec2 punto(4.0f + desplazamientoFinn.x, 22.0f + desplazamientoFinn.z);
        float limite = glm::max(1.0f, radioCaminable - 2.0f);
        if (glm::dot(punto, punto) > limite * limite)
        {
            punto = glm::normalize(punto) * limite;
            desplazamientoFinn.x = punto.x - 4.0f;
            desplazamientoFinn.z = punto.y - 22.0f;
        }

        // El tronco y las rocas grandes bloquean el paso; las flores no.
        glm::vec2 libre = CorregirPosicionLocalOoo(
            glm::vec2(4.0f + desplazamientoFinn.x, 22.0f + desplazamientoFinn.z), .65f);
        desplazamientoFinn.x = libre.x - 4.0f;
        desplazamientoFinn.z = libre.y - 22.0f;
        float recorrido = glm::length(desplazamientoFinn - anterior);
        caminandoFinn = recorrido > 0.00001f;
        if (caminandoFinn)
        {
            faseCaminataFinn = std::fmod(faseCaminataFinn + recorrido * FASE_POR_UNIDAD,
                2.0f * 3.14159265f);
        }

        // Girar hacia donde va. atan2 evita los brincos de -180 a 180 grados.
        float destino = std::atan2(direccion.x, direccion.z);
        float actual = glm::radians(rumboFinn);
        float diferencia = std::atan2(std::sin(destino - actual), std::cos(destino - actual));
        rumboFinn = glm::degrees(actual + diferencia * (1.0f - std::exp(-10.0f * dt)));
    }

    // Arranque y frenado cortos para evitar que las extremidades brinquen.
    float objetivo = caminandoFinn ? 1.0f : 0.0f;
    intensidadCaminataFinn += (objetivo - intensidadCaminataFinn) * (1.0f - std::exp(-13.0f * dt));
}

void IniciarSaludoFinn(float tiempo)
{
    // G inicia un saludo desde el principio, incluso si ya estaba animado.
    saludoIniciado = true;
    inicioSaludo = tiempo;
}

void DibujarFinn(const glm::vec3& posicion, float tiempo)
{
    glm::mat4 torso = glm::translate(glm::mat4(1.0f), posicion + desplazamientoFinn);
    torso = glm::rotate(torso, glm::radians(rumboFinn), glm::vec3(0.0f, 1.0f, 0.0f));
    torso = glm::scale(torso, glm::vec3(ALTURA_FINN / ALTURA_ORIGINAL));
    torso = glm::translate(torso, glm::vec3(0.0f, -BASE_ORIGINAL, 0.0f));

    // La fase avanza solo al recorrer distancia; en reposo vuelve a su postura.
    float paso = intensidadCaminataFinn * std::sin(faseCaminataFinn);
    float tSaludo = saludoIniciado ? tiempo - inicioSaludo : -1.0f;
    float saludo = IntensidadSaludo(tSaludo);
    // Durante el saludo solo se aparta el brazo derecho de la caminata.
    float brazoDer = 28.0f * paso * (1.0f - saludo);
    float brazoIzq = -28.0f * paso;
    float musloDer = -22.0f * paso;
    float musloIzq = 22.0f * paso;
    float antebrazoDer = intensidadCaminataFinn * (-18.0f - 12.0f * std::sin(faseCaminataFinn + 0.7f));
    antebrazoDer *= (1.0f - saludo);

    // El brazo sube por el hombro; el codo sigue al hombro y mueve la mano.
    float hombroSaludo = -100.0f * saludo;
    float codoSaludo = (-55.0f + 18.0f * std::sin((tSaludo - SUBIR_BRAZO) * 10.0f)) * saludo;
    float antebrazoIzq = intensidadCaminataFinn * (-18.0f + 12.0f * std::sin(faseCaminataFinn + 0.7f));
    float rodillaDer = 25.0f * std::fmax(0.0f, paso);
    float rodillaIzq = 25.0f * std::fmax(0.0f, -paso);

    UsarMaterial(matOpaco);
    DibujarModelo(piezas[TORSO], torso);

    glm::mat4 cabeza = glm::translate(torso, CUELLO);
    float giroCabeza = 5.0f * intensidadCaminataFinn * std::sin(faseCaminataFinn * 0.5f);
    cabeza = glm::rotate(cabeza, glm::radians(giroCabeza), glm::vec3(0.0f, 1.0f, 0.0f));
    cabeza = glm::rotate(cabeza, glm::radians(-4.0f * saludo), glm::vec3(0.0f, 0.0f, 1.0f));
    DibujarModelo(piezas[CABEZA], glm::translate(cabeza, -CUELLO));

    DibujarBrazo(torso, BRAZO_DER, ANTEBRAZO_DER, HOMBRO_DER, CODO_DER,
        brazoDer, antebrazoDer, hombroSaludo, codoSaludo);
    DibujarBrazo(torso, BRAZO_IZQ, ANTEBRAZO_IZQ, HOMBRO_IZQ, CODO_IZQ, brazoIzq, antebrazoIzq);
    DibujarPierna(torso, MUSLO_DER, PIERNA_DER, CADERA_DER, RODILLA_DER, musloDer, rodillaDer);
    DibujarPierna(torso, MUSLO_IZQ, PIERNA_IZQ, CADERA_IZQ, RODILLA_IZQ, musloIzq, rodillaIzq);
}
