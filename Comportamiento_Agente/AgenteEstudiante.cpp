#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
        Status(tablero, mejor);
        return mejor;
        break;    

    case ModoJuego::MINIMAX:
        minimax(tablero, 0, profundidadMax, mejor);
        return mejor;
        break; 

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    /* ============== Empieza a partir de aquí tu implementación  =============== */
    
    Mov={-1, -1};
    //en comprobarGanador se analiza la victoria/derrota por desempate
    int ganador = tablero.comprobarGanador();
    
    //victoria, derrota, empate directo (no actualizamos Mov)
    int oponente = (id == 1) ? 2 : 1;
    if (ganador == id)  return Resultado::VICTORIA;
    else if (ganador == oponente) return Resultado::DERROTA;
    else if (ganador == -1) return Resultado::EMPATE;

    auto sucesores = tablero.getSucesoresConMovimientos();
    if (sucesores.empty()){
        Mov={-1, -1}; // No hay movimientos posibles, aunque esto no debería ocurrir si el tablero no tiene ganador ni empate
        return Resultado::EMPATE; // Si no hay sucesores, es un empate por falta de movimientos
    }
    Mov = sucesores[0].second; // Inicializo Mov con el primer movimiento posible para asegurar que siempre se devuelve algo válido
     
    bool esMiTurno = (tablero.getJugadorTurno() == id);

    Resultado mejorResultado = esMiTurno ? Resultado::DERROTA : Resultado::VICTORIA;
    for (const auto& [tableroSucesor, movimientoSucesor] : sucesores) {
        std::pair<int,int> movTemp;
        Resultado res = Status(tableroSucesor, movTemp);

        if (esMiTurno) {
            //MAX: Busco la mejor opción para mí
            if (res == Resultado::VICTORIA) {
                Mov = movimientoSucesor;
                return Resultado::VICTORIA; // Si encuentro una victoria, la elijo inmediatamente, no sigo explorando
            }
            else if (res == Resultado::EMPATE && mejorResultado != Resultado::VICTORIA) {
                mejorResultado = Resultado::EMPATE; // Mejor que ganar para el oponente
                Mov = movimientoSucesor;
            }
            // MIN: El rival busca el peor resultado para mí
        } else {
            if (res == Resultado::DERROTA) {
                Mov = movimientoSucesor;
                return Resultado::DERROTA; // Si el oponente puede llevarme a derrota, la eligirá
            }
            else if (res == Resultado::EMPATE && mejorResultado != Resultado::DERROTA) {
                mejorResultado = Resultado::EMPATE; // El rival prefiere empatar antes que dejarme ganar
                Mov = movimientoSucesor;
            }
        }
    }

    return mejorResultado;
}



/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    //1. Comprobar que el tablero es un estado terminal (ganador, perdedor o empate) y devolver el valor correspondiente
    int ganador = tablero.comprobarGanador();
    int oponente = (id == 1) ? 2 : 1;

    if (ganador == id) return GANAR;
    if (ganador == oponente) return PERDER;
    if (ganador == -1) return 0;

    //2. Si se ha alcanzado la profundidad máxima, evaluar el tablero con la función heurística
    if (profundidad == prof_Max) {
        return heuristica(tablero);
    }

    //3. Generar los tableros sucesores y aplicar minimax recursivamente
    auto sucesores = tablero.getSucesoresConMovimientos();

    if (sucesores.empty()) {
        Mov = {-1, -1};
        return heuristica(tablero); // Si no hay sucesores, evalúo el tablero actual
    }

    bool esMiTurno = tablero.getJugadorTurno() == id;
    double mejorValor = MenosInfinito;
    Mov = sucesores[0].second;

    //4. Si es el turno del agente, buscar el valor máximo
    if (esMiTurno) {

        for (const auto& [tableroSucesor, movimientoSucesor] : sucesores) {
            std::pair<int,int> movTemp;
            double valor = minimax(tableroSucesor, profundidad + 1, prof_Max, movTemp);

            if (abortarBanda) return 0;

            if (valor > mejorValor) {
                mejorValor = valor;
                Mov = movimientoSucesor;
            }
        }

        
    //5. Si es el turno del oponente, buscar el valor mínimo
    } else {
        mejorValor = MasInfinito;

        for (const auto& [tableroSucesor, movimientoSucesor] : sucesores) {
            std::pair<int,int> movTemp;
            double valor = minimax(tableroSucesor, profundidad + 1, prof_Max, movTemp);

            if (abortarBanda) return 0;

            if (valor < mejorValor) {
                mejorValor = valor;
                Mov = movimientoSucesor;
            }
        }

    }
    return mejorValor;
}


/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov;

    double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}




/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    // 1. Comprobar estados terminales
    int ganador = tablero.comprobarGanador();
    int oponente = (id == 1) ? 2 : 1;

    if (ganador == id) return GANAR;
    if (ganador == oponente) return PERDER;
    if (ganador == -1) return 0;

    // 2. Si alcanzamos profundidad máxima, usamos heurística
    if (profundidad == prof_Max) {
        return heuristica(tablero);
    }

    // 3. Generar sucesores
    auto sucesores = tablero.getSucesoresConMovimientos();

    if (sucesores.empty()) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    bool esMiTurno = tablero.getJugadorTurno() == id;
    Mov = sucesores[0].second;
    double mejorValor = MenosInfinito;

    // 4. MAX: turno de mi agente
    if (esMiTurno) {
        

        for (const auto& [tableroSucesor, movimientoSucesor] : sucesores) {
            std::pair<int,int> movTemp;
            double valor = alfaBeta(tableroSucesor, profundidad + 1, prof_Max, alfa, beta, movTemp);

            if (abortarBanda) return 0;

            if (valor > mejorValor) {
                mejorValor = valor;
                Mov = movimientoSucesor;
            }

            alfa = std::max(alfa, mejorValor);

            if (beta <= alfa) break; //Poda beta, no necesito seguir explorando este nodo
        }
 
    }

    // 5. MIN: turno del rival
    else {
        mejorValor = MasInfinito;

        for (const auto& [tableroSucesor, movimientoSucesor] : sucesores) {
            std::pair<int,int> movTemp;
            double valor = alfaBeta(tableroSucesor, profundidad + 1, prof_Max, alfa, beta, movTemp);

            if (abortarBanda) return 0;

            if (valor < mejorValor) {
                mejorValor = valor;
                Mov = movimientoSucesor;
            }

            beta = std::min(beta, mejorValor);

            if (beta <= alfa) break;
        }

    }
    return mejorValor;
}

/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        case 2: return heuristica2(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                  score_positivo += valor;
                 }
                else {
                  score_negativo += valor;
                }
            }
        }
    }

   
    return score_positivo - score_negativo;
}



double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    int ganador = tablero.comprobarGanador();
    int oponente = (id == 1) ? 2 : 1;

    if (ganador == id) return GANAR;
    if (ganador == oponente) return PERDER;
    if (ganador == -1) return 0;

    int n = tablero.getNParaGanar();
    int filas = tablero.getFilas();
    int columnas = tablero.getColumnas();
    double puntos = 0.0;

    // Lineas generales: se penaliza mas al rival para favorecer el bloqueo.
    for (int longitud = 2; longitud < n; ++longitud) {
        double peso = std::pow(10.0, longitud);

        int misLineas = tablero.contarCombinaciones(longitud, id);
        int susLineas = tablero.contarCombinaciones(longitud, oponente);

        puntos += peso * misLineas;
        puntos -= peso * 1.5 * susLineas;
    }

    // Amenazas fuertes: lineas casi ganadoras.
    int misAmenazas = tablero.contarCombinaciones(n - 1, id);
    int amenazasRival = tablero.contarCombinaciones(n - 1, oponente);

    puntos += 60000.0 * misAmenazas;
    puntos -= 120000.0 * amenazasRival;

    // Pre-amenazas: lineas que pueden convertirse pronto en amenaza directa.
    if (n > 3) {
        int misPreAmenazas = tablero.contarCombinaciones(n - 2, id);
        int preAmenazasRival = tablero.contarCombinaciones(n - 2, oponente);

        puntos += 8000.0 * misPreAmenazas;
        puntos -= 12000.0 * preAmenazasRival;
    }

    int centroF = filas / 2;
    int centroC = columnas / 2;

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            int celda = tablero.getCelda(f, c);
            Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);

            if (celda != 0) {
                double valorCentro = filas + columnas - std::abs(f - centroF) - std::abs(c - centroC);

                if (celda == id) puntos += valorCentro * 2.0;
                else if (celda == oponente) puntos -= valorCentro * 2.0;

                if (tipo == Tablero::TipoCelda::VERDE) {
                    if (celda == id) puntos += 150.0;
                    else if (celda == oponente) puntos -= 150.0;
                }
            } else {
                if (tipo == Tablero::TipoCelda::VERDE) {
                    puntos += 80.0;
                } else if (tipo == Tablero::TipoCelda::ROJO) {
                    puntos -= 120.0;
                } else if (tipo == Tablero::TipoCelda::AMARILLO) {
                    puntos += 30.0;
                }
            }
        }
    }

    return puntos;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
    int ganador = tablero.comprobarGanador();
    int oponente = (id == 1) ? 2 : 1;

    if (ganador == id) return GANAR;
    if (ganador == oponente) return PERDER;
    if (ganador == -1) return 0;

    int n = tablero.getNParaGanar();
    double puntos = 0.0;

    // Combinaciones de longitud 2: utiles, pero no decisivas.
    if (n > 2) {
        int mias2 = tablero.contarCombinaciones(2, id);
        int rival2 = tablero.contarCombinaciones(2, oponente);

        puntos += 10 * mias2;
        puntos -= 15 * rival2;
    }

    // Combinaciones de longitud 3: empiezan a ser amenazas importantes.
    if (n > 3) {
        int mias3 = tablero.contarCombinaciones(3, id);
        int rival3 = tablero.contarCombinaciones(3, oponente);

        puntos += 120 * mias3;
        puntos -= 180 * rival3;
    }

    // Combinaciones de longitud 4: muy importantes si se juega a 5 en raya.
    if (n > 4) {
        int mias4 = tablero.contarCombinaciones(4, id);
        int rival4 = tablero.contarCombinaciones(4, oponente);

        puntos += 1500 * mias4;
        puntos -= 3000 * rival4;
    }

    int misAmenazas = tablero.contarCombinaciones(n - 1, id);
    int amenazasRival = tablero.contarCombinaciones(n - 1, oponente);

    puntos += 5000 * misAmenazas;
    puntos -= 8000 * amenazasRival;

    return puntos;
}