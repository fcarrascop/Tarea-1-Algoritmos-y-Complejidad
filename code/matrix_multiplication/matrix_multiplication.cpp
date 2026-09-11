/*
 * INF-221 TAREA 1 2026-2 — Algoritmos y Complejidad
 * Programa principal de mediciones para el problema de multiplicacion de matrices.
 * Autor: Felipe Carrasco P, rol 202473522-5
 *
 * Uso:
 * ./matrix_multiplication <algoritmo> <archivo_M1> <archivo_M2> <archivo_salida| - >
 *
 *  <algoritmo> : naive | strassen
 *  <archivo_M1> : ruta a data/matrix_input/{n}_{t}_{d}_{m}_1.txt
 *  <archivo_M2> : ruta a data/matrix_input/{n}_{t}_{d}_{m}_2.txt
 *  <archivo_salida> : ruta donde escribir el producto, o "-" para no escribir
 *
 * Salida (stdout): una linea CSV -> algoritmo,n,tipo,dominio,muestra,tiempo_ms,memoria_kb,maquina
 *
 * Metodologia identica a code/sorting/sorting.cpp:
 * - Un proceso por medicion.
 * - Tiempo: solo la llamada al algoritmo; se repite hasta ~200 ms para n chico.
 * - Memoria: pico de RSS menos linea base (tomada tras cargar M1 y M2), es decir la matriz resultado mas la memoria de trabajo del algoritmo.
 */

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <unistd.h>

#include "algorithms/matmul.hpp"

using Clock = std::chrono::steady_clock;

static long peak_rss_kb() {
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
#ifdef __APPLE__
    return r.ru_maxrss / 1024; // macOS
#else
    return r.ru_maxrss; // Linux
#endif
}

// "data/matrix_input/16_densa_D0_a_1.txt" -> ("16","densa","D0","a")
static void parse_nombre(const std::string& path, std::string out[4]) {
    size_t slash = path.find_last_of("/\\");
    std::string base = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = base.rfind(".txt");
    if (dot != std::string::npos) base.resize(dot);
    if (base.size() >= 2 && base[base.size() - 2] == '_')
        base.resize(base.size() - 2);

    for (int i = 0; i < 4; ++i) out[i].clear();
    int campo = 0;
    for (char c : base) {
        if (c == '_' && campo < 3) { ++campo; continue; }
        out[campo].push_back(c);
    }
}

static Matrix leer_matriz(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "no puedo abrir %s\n", path.c_str());
        std::exit(1);
    }
    Matrix m;
    std::string linea;
    while (std::getline(in, linea)) {
        std::istringstream ss(linea);
        std::vector<int> fila;
        int x;
        while (ss >> x) fila.push_back(x);
        if (!fila.empty()) m.push_back(std::move(fila));
    }
    return m;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::fprintf(stderr, "uso: %s <algoritmo> <M1> <M2> <salida|->\n", argv[0]);
        return 1;
    }
    const std::string algo = argv[1];
    const std::string p1 = argv[2], p2 = argv[3], out_path = argv[4];

    Matrix (*mult_fn)(const Matrix&, const Matrix&) = nullptr;
    if (algo == "naive")         mult_fn = naive_multiply;
    else if (algo == "strassen") mult_fn = strassen_multiply;
    else {
        std::fprintf(stderr, "algoritmo desconocido: %s\n", algo.c_str());
        return 1;
    }

    const Matrix A = leer_matriz(p1);
    const Matrix B = leer_matriz(p2);
    if (A.empty() || B.empty() || A.size() != B.size()) {
        std::fprintf(stderr, "matrices invalidas o de distinto tamano: %s %s\n",
                     p1.c_str(), p2.c_str());
        return 1;
    }

    const long base_kb = peak_rss_kb();

    // tiempo
    long reps = 0;
    double total_ms = 0.0;
    Matrix C;
    const auto wall0 = Clock::now();
    do {
        const auto t0 = Clock::now();
        C = mult_fn(A, B);
        const auto t1 = Clock::now();
        total_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
        ++reps;
    } while (std::chrono::duration<double, std::milli>(Clock::now() - wall0).count() < 200.0 && reps < 10000);

    const long peak_kb = peak_rss_kb();
    const double time_ms = total_ms / reps;
    long mem_kb = peak_kb - base_kb;
    if (mem_kb < 0) mem_kb = 0;

    // validar que el resultado sea igual en los dos algoritmos
    if (algo == "strassen" && C != naive_multiply(A, B)) {
        std::fprintf(stderr, "ERROR: strassen != naive en %s\n", p1.c_str());
        return 2;
    }

    // escribir producto
    if (out_path != "-") {
        std::ofstream out(out_path);
        for (const auto& fila : C) {
            for (size_t j = 0; j < fila.size(); ++j)
                out << fila[j] << (j + 1 < fila.size() ? ' ' : '\n');
        }
    }

    // entrada CSV
    std::string f[4];
    parse_nombre(p1, f);
    char host[256] = "desconocida";
    gethostname(host, sizeof(host));
    std::printf("%s,%s,%s,%s,%s,%.6f,%ld,%s\n",
                algo.c_str(), f[0].c_str(), f[1].c_str(), f[2].c_str(), f[3].c_str(),
                time_ms, mem_kb, host);
    return 0;
}
