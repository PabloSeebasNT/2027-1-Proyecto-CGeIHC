#include "OooEvolucion.h"
#include "Geometria.h"
#include "PlanetaOoo.h"
#include <gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace {
    constexpr float PI = 3.14159265f, DURACION = 2.8f;
    int etapa = 0, destino = 0;
    float inicio = -1;
    Texture* blanca = nullptr;
    MeshModel* franjas[4] = {};
    Material mate(.12f, 12);

    const glm::vec3 AZUL(.57f, .82f, .92f), ROSA(.96f, .56f, .71f);
    const glm::vec3 CREMA(.98f, .96f, .84f), TINTA(.065f, .055f, .09f);
    const glm::vec3 AMARILLO(.98f, .85f, .43f), MENTA(.64f, .84f, .41f);
    const glm::vec3 VIOLETA(.72f, .63f, .91f), TURQUESA(.30f, .72f, .78f);
    const glm::vec3 CAPA_MORADO(.62f, .48f, .83f), CAPA_LILA(.78f, .67f, .95f);
    const glm::vec3 CAPA_TURQ(.36f, .73f, .63f), CAPA_TURQ2(.27f, .56f, .66f);
    const glm::vec3 alasColA(.96f, .94f, 1.0f), alasColB(.88f, .72f, .96f);

    const glm::vec3 colores[] = { AZUL, AMARILLO, MENTA, ROSA };

    struct Forma {
        float tamano, cuerno, madurez, poder, caparazon, alas;
    };
    const Forma formas[] = {
        {1.18f, .40f, 0, 0, 0, 0},
        {1.52f, .43f, 1, 0, 1, 0},
        {1.82f, .66f, 2, 1, 1, 1}
    };

    float Suave(float x) {
        x = glm::clamp(x, 0.0f, 1.0f);
        return x * x * x * (x * (6 * x - 15) + 10);
    }

    void Ovo(const glm::mat4& p, glm::vec3 lugar, glm::vec3 escala, glm::vec3 color) {
        Dibujar(mEsfera, glm::scale(glm::translate(p, lugar), escala), color, .045f);
    }

    void Tramo(const glm::mat4& p, glm::vec3 a, glm::vec3 b, float radio, glm::vec3 color) {
        glm::vec3 d = b - a;
        float largo = glm::length(d);
        if (largo < .0001f) return;
        glm::mat4 m = glm::translate(p, (a + b) * .5f);
        glm::vec3 eje = d / largo;
        glm::vec3 cruz = glm::cross(glm::vec3(0, 1, 0), eje);
        if (glm::length(cruz) > .001f)
            m = glm::rotate(m, std::acos(glm::clamp(eje.y, -1.0f, 1.0f)), glm::normalize(cruz));
        else if (eje.y < 0)
            m = glm::rotate(m, PI, glm::vec3(1, 0, 0));
        Dibujar(mCilindro, glm::scale(m, { radio, largo, radio }), color, .045f);
    }

    MeshModel* Banda(float inicioLat, float finLat) {
        std::vector<GLfloat> v;
        std::vector<unsigned int> idx;
        const int filas = 8, lados = 32;
        for (int i = 0; i <= filas; ++i)
            for (int j = 0; j <= lados; ++j) {
                float lat = glm::mix(inicioLat, finLat, float(i) / filas), lon = 2 * PI * j / lados;
                glm::vec3 p(std::sin(lat) * std::cos(lon), std::cos(lat), std::sin(lat) * std::sin(lon));
                AgregarVertice(v, p, float(j) / lados, lat / PI, p);
                if (i < filas && j < lados) {
                    unsigned k = i * (lados + 1) + j;
                    idx.insert(idx.end(), { k, k + lados + 1, k + 1, k + 1, k + lados + 1, k + lados + 2 });
                }
            }
        return GuardarMalla(v, idx);
    }

    glm::vec3 MezclaEtapas(glm::vec3 base, glm::vec3 juvenil, glm::vec3 final, float madurez) {
        glm::vec3 color = glm::mix(base, juvenil, glm::clamp(madurez, 0.0f, 1.0f));
        return glm::mix(color, final, glm::clamp(madurez - 1, 0.0f, 1.0f));
    }

    void DibujarCaparazon(const glm::mat4& local, const Forma& f, float t) {
        if (f.caparazon <= .001f) return;
        glm::mat4 cap=glm::scale(local,glm::vec3(f.caparazon));
        const glm::vec3 azul(.08f,.37f,.84f),borde(.83f,.91f,.98f),linea(.025f,.09f,.21f);
        // La cara mira a +Z; el caparazon se apoya en la espalda, hacia -Z.
        Ovo(cap,{0,.03f,-.64f},{1.09f,1.04f,.30f},borde);
        Ovo(cap,{0,.03f,-.73f},{1.01f,.97f,.58f},azul);
        // Proyectar cada punto sobre el domo evita costuras flotantes o enterradas.
        auto superficie=[](glm::vec2 p){
            float interior=std::max(0.0f,1-p.x*p.x/(1.01f*1.01f)-p.y*p.y/(.97f*.97f));
            return glm::vec3(p.x,p.y+.03f,-.73f-.58f*std::sqrt(interior)-.012f);
        };
        auto costura=[&](glm::vec2 a,glm::vec2 b){
            for(int k=0;k<12;++k)
                Tramo(cap,superficie(glm::mix(a,b,k/12.0f)),superficie(glm::mix(a,b,(k+1)/12.0f)),.018f,linea);
        };
        // Placa central hexagonal y seis placas exteriores, como la referencia.
        for(int i=0;i<6;++i){
            float a=PI/6+i*PI/3,b=a+PI/3;
            glm::vec2 p(.48f*std::cos(a),.48f*std::sin(a));
            glm::vec2 q(.48f*std::cos(b),.48f*std::sin(b));
            costura(p,q);
            costura(p,{.95f*std::cos(a),.91f*std::sin(a)});
        }
        if(f.alas<=.001f)return;
        for(float s:{-1.0f,1.0f}){
            glm::mat4 ala=glm::translate(cap,{s*.86f,.26f,-.93f});
            ala=glm::rotate(ala,glm::radians(s*(18+22*std::sin(t*3.5f))),{0,0,1});
            ala=glm::scale(ala,glm::vec3(f.alas));
            // Union -> ala -> plumas: todas heredan el mismo pivote de aleteo.
            Ovo(ala,{0,0,0},{.16f,.17f,.16f},borde);
            Ovo(ala,{s*.40f,.12f,0},{.55f,.24f,.14f},{.56f,.78f,.98f});
            for(int j=0;j<4;++j){
                glm::mat4 pluma=glm::translate(ala,{s*(.50f+j*.12f),.06f+j*.12f,-.025f*j});
                pluma=glm::rotate(pluma,glm::radians(s*(15+11*j)),{0,0,1});
                Ovo(pluma,{s*.32f,0,0},{.49f-.035f*j,.12f,.105f},glm::mix(borde,glm::vec3(.53f,.77f,.98f),j*.16f));
            }
        }
    }

    void Cabeza(const glm::mat4& padre, glm::vec3 escala, float tiempo, const Forma& f) {
        glm::mat4 cabeza = glm::scale(padre, escala);
        const glm::vec3 juvenil[] = { VIOLETA, AMARILLO, glm::vec3(.48f, .78f, .71f), glm::vec3(.91f, .47f, .66f) };
        const glm::vec3 final[] = { TURQUESA, glm::vec3(1.0f, .83f, .42f), glm::vec3(.68f, .55f, .90f), glm::vec3(.57f, .40f, .76f) };
        for (int i = 0; i < 4; ++i) {
            glm::vec3 color = MezclaEtapas(colores[i], juvenil[i], final[i], f.madurez);
            Dibujar(franjas[i], cabeza, color, .06f);
        }

        glm::vec3 piel = MezclaEtapas(AZUL, juvenil[0], final[0], f.madurez);
        float ciclo = std::fmod(std::max(0.0f, tiempo), 5.3f);
        float ojo = ciclo > 5.05f ? std::max(.08f, std::abs(ciclo - 5.175f) / .125f) : 1;
        for (float s : { -1.0f, 1.0f }) {
            Ovo(cabeza, { s * .38f, .24f, .91f }, { .245f, .27f * ojo, .09f }, TINTA);
            Ovo(cabeza, { s * .38f, .25f, .976f }, { .198f, .215f * ojo, .035f }, CREMA);
            Ovo(cabeza, { s * .33f, .24f, 1.006f }, { .065f, .10f * ojo, .023f }, TINTA);
            Ovo(cabeza, { s * .73f, -.08f, .69f }, { .13f, .075f, .04f }, ROSA);
            Ovo(cabeza, { s * .94f, .18f, 0 }, { .13f, .30f, .22f }, piel);
        }
        Ovo(cabeza, { 0, .08f, 1.01f }, { .13f, .085f, .065f }, TINTA);
        for (float s : { -1.0f, 1.0f })
            for (int i = 0; i < 8; ++i) {
                float a = float(i) / 8, b = float(i + 1) / 8;
                glm::vec3 p(s * .22f * a, -.015f - .10f * std::sin(PI * a), 1.012f - .025f * a);
                glm::vec3 q(s * .22f * b, -.015f - .10f * std::sin(PI * b), 1.012f - .025f * b);
                Tramo(cabeza, p, q, .022f, TINTA);
            }
        Dibujar(mCono, glm::scale(glm::translate(cabeza, { 0, .94f, 0 }), { .14f + .025f * f.poder, f.cuerno, .14f + .025f * f.poder }), CREMA, .04f + .18f * f.poder);
        if (f.poder > .001f)
            for (int i = 0; i < 18; ++i) {
                float edad = std::fmod(std::max(0.0f, tiempo) * .55f + i / 18.0f, 1.0f);
                float angulo = i * 2.39996f + edad * 3.1f, radio = .35f * edad;
                glm::vec3 pos(radio * std::cos(angulo), .94f + f.cuerno + .60f * edad, radio * std::sin(angulo));
                float tam = .040f * f.poder * std::sin(PI * edad);
                if (tam > .001f)
                    Dibujar(mEsferaBaja, glm::scale(glm::translate(cabeza, pos), glm::vec3(tam)),
                        glm::mix(CREMA, glm::vec3(.60f, .92f, 1), edad), .6f * f.poder);
            }
    }

    Forma Mezclar(const Forma& a, const Forma& b, float u) {
        return {
            glm::mix(a.tamano, b.tamano, u),
            glm::mix(a.cuerno, b.cuerno, u),
            glm::mix(a.madurez, b.madurez, u),
            glm::mix(a.poder, b.poder, u),
            glm::mix(a.caparazon, b.caparazon, u),
            glm::mix(a.alas, b.alas, u)
        };
    }

    void Criatura(const Forma& f, float t) {
        float altura = f.tamano * 1.10f + f.poder * (.26f + .10f * std::sin(t * 1.65f));
        glm::mat4 raiz = glm::translate(glm::mat4(1), CENTRO_OOO + glm::vec3(43, .12f + altura, -33));
        raiz = glm::rotate(raiz, glm::radians((2 + 3 * f.poder) * std::sin(t * .7f)), { 0, 1, 0 });
        glm::mat4 cuerpo = glm::rotate(raiz, glm::radians(1.5f * std::sin(t * 1.15f)), { 0, 0, 1 });

        Cabeza(cuerpo, { f.tamano, f.tamano * .96f, f.tamano * .88f }, t, f);
        glm::mat4 local = glm::scale(cuerpo, glm::vec3(f.tamano));

        // Patas base.
        for (float s : { -1.0f, 1.0f }) {
            glm::mat4 pie = glm::translate(local, { s * .43f, -.94f, .12f });
            pie = glm::rotate(pie, glm::radians(6 * f.poder * std::sin(t * 2 + s)), { 1, 0, 0 });
            Ovo(pie, { 0, -.06f, .05f }, { .21f, .14f, .27f }, ROSA);

            glm::mat4 hombro = glm::translate(local, { s * .80f, -.02f, .40f });
            hombro = glm::rotate(hombro, glm::radians(s * (12 + 7 * std::sin(t * 1.3f + s))), { 0, 0, 1 });
            Tramo(hombro, { 0, 0, 0 }, { 0, -.24f, .08f }, .065f, CREMA);
            glm::mat4 codo = glm::translate(hombro, { 0, -.24f, .08f });
            codo = glm::rotate(codo, glm::radians(-25 - 10 * f.poder * std::sin(t * 1.7f + s)), { 1, 0, 0 });
            Tramo(codo, { 0, 0, 0 }, { 0, -.17f, .08f }, .055f, CREMA);
            Ovo(codo, { 0, -.17f, .08f }, { .09f, .10f, .09f }, CREMA);
        }

        // Patas traseras extra desde la segunda etapa.
        if (f.caparazon > .15f) {
            for (float s : { -1.0f, 1.0f }) {
                float paso = std::sin(t * 1.45f + (s > 0 ? 0.8f : 2.1f));
                glm::mat4 cadera = glm::translate(local, { s * .52f, -.36f, -.34f });
                cadera = glm::rotate(cadera, glm::radians(s * (8 + 10 * paso * f.caparazon)), { 0, 0, 1 });
                Tramo(cadera, { 0, 0, 0 }, { 0, -.28f, -.02f }, .060f, CREMA);
                glm::mat4 rodilla = glm::translate(cadera, { 0, -.28f, -.02f });
                rodilla = glm::rotate(rodilla, glm::radians(-18 - 18 * paso * f.caparazon), { 1, 0, 0 });
                Tramo(rodilla, { 0, 0, 0 }, { 0, -.22f, .10f }, .050f, CREMA);
                Ovo(rodilla, { 0, -.22f, .10f }, { .10f, .09f, .11f }, ROSA);
            }
        }

        DibujarCaparazon(local, f, t);
    }
}

void CargarEvolucionOoo() {
    blanca = CargarTextura("Textures/plain.png");
    const float lat[] = { 0, 1.64f, 1.84f, 2.03f, PI };
    for (int i = 0; i < 4; ++i) franjas[i] = Banda(lat[i], lat[i + 1]);
}

void FijarEvolucionOoo(int origen, int siguiente, float comienzo) {
    etapa = std::max(0, std::min(2, origen));
    destino = std::max(0, std::min(2, siguiente));
    inicio = comienzo;
}

void AvanzarEvolucionOoo(float tiempo) {
    if (inicio >= 0 && tiempo - inicio < DURACION) return;
    if (inicio >= 0) etapa = destino;
    destino = (etapa + 1) % 3;
    inicio = tiempo;
    printf("Ooo: cambio de variante %d -> %d (J)\n", etapa + 1, destino + 1);
}

void DibujarEvolucionOoo(float tiempo) {
    if (!blanca) return;
    blanca->UseTexture();
    UsarMaterial(mate);
    float u = inicio < 0 ? 0 : Suave((tiempo - inicio) / DURACION);
    Criatura(Mezclar(formas[etapa], formas[destino], u), tiempo);
}
