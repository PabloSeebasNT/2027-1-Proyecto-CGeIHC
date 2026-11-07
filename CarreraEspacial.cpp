#include "CarreraEspacial.h"
#include "CircuitoEspacial.h"
#include "NaveVerde.h"
#include <cmath>

namespace {
    struct Corredor { float carril, velocidad, salida, fase; };
    const Corredor corredores[] = {
        {-2.6f, 16.0f, -8.0f, 0.0f},
        { 2.6f, 16.4f, -14.0f, 2.4f}
    };
    const float ESCALA = .36f;

    float Distancia(const Corredor& nave, float t) {
        // Integral de v(t) = velocidad + 2*cos(.22*t + fase).
        // Evaluar por tiempo evita acumular errores segun los FPS.
        return nave.salida + nave.velocidad*t
            + 2.0f/.22f*(std::sin(.22f*t+nave.fase)-std::sin(nave.fase));
    }

    glm::vec3 PuntoCarril(float distancia, float carril) {
        glm::vec3 p, frente;
        PoseCircuitoEspacial(distancia, p, frente);
        glm::vec3 lado = glm::normalize(glm::cross(glm::vec3(0,1,0), frente));
        glm::vec3 arriba = glm::normalize(glm::cross(frente, lado));
        return p + lado*carril + arriba*1.65f;
    }
}

void PoseCorredor(int corredor, float tiempo, glm::vec3& posicion,
    glm::vec3& frente, glm::vec3& techo) {
    const auto& nave = corredores[corredor == 1 ? 1 : 0];
    float s = Distancia(nave, tiempo);
    posicion = PuntoCarril(s, nave.carril);
    auto antes = PuntoCarril(s-.6f, nave.carril);
    auto despues = PuntoCarril(s+.6f, nave.carril);
    frente = glm::normalize(despues-antes);
    auto lado = glm::normalize(glm::cross(glm::vec3(0,1,0), frente));
    techo = glm::normalize(glm::cross(frente, lado));

    // Se inclina hacia el interior de la curva, con un limite moderado.
    auto curva = glm::normalize(despues-posicion)-glm::normalize(posicion-antes);
    float giro = glm::clamp(glm::dot(curva,lado)*2.5f, -.22f, .22f);
    techo = glm::normalize(techo + lado*giro);
    // Flotacion ligera; no mueve la trayectoria ni altera la velocidad de avance.
    posicion.y += .10f*std::sin(2.1f*tiempo+nave.fase);
}

void DibujarCarreraEspacial(float tiempo) {
    for(int i=0;i<2;++i) {
        glm::vec3 p, frente, techo;
        PoseCorredor(i, tiempo, p, frente, techo);
        float potencia = .70f + .10f*std::cos(.22f*tiempo+corredores[i].fase);
        // Reutiliza toda la jerarquia: alas, motores, radar y tren plegado.
        DibujarNaveVerde(MatrizDesdeEjes(p,frente,techo,ESCALA),
            0.0f,potencia,tiempo+corredores[i].fase);
    }
}
