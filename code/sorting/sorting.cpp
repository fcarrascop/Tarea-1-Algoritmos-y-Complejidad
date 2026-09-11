/*
 * INF-221 TAREA 1 2026-2 — Algoritmos y Complejidad
 * Programa principal de mediciones para el problema de ordenamiento.
 * Autor: Felipe Carrasco P, rol 202473522-5
 *
 * Uso:
 * ./sorting <algoritmo> <archivo_entrada> <archivo_salida|->
 *
 * <algoritmo> : mergesort | quicksort | patiencesort | stdsort
 * <archivo_entrada> : ruta a data/array_input/{n}_{t}_{d}_{m}.txt
 * <archivo_salida> : ruta donde escribir el arreglo ordenado, o "-" para no escribir
 *
 * Salida (stdout): una linea CSV -> algoritmo,n,tipo,dominio,muestra,tiempo_ms,memoria_kb,maquina
 *
 * Metodologia:
 * - Un proceso por medicion (el makefile invoca este binario una vez por cada combinacion) para que el pico de memoria y el tiempo de un algoritmo no contaminen al siguiente.
 * - Tiempo: se cronometra solo la llamada al algoritmo (sin E/S). Para entradas chicas se repite la corrida hasta ~200 ms y se promedia.
 * - Memoria: pico de RSS del proceso (high-water mark del kernel) menos una linea base tomada con el buffer de trabajo ya reservado, de modo que la cifra reportada es la memoria auxiliar del algoritmo.
 */

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <unistd.h>

#include "algorithms/algorithms.hpp"

using Clock = std::chrono::steady_clock;

// Pico de RSS del proceso, en kilobytes.
static long peak_rss_kb() {
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
#ifdef __APPLE__
    return r.ru_maxrss / 1024; // macOS
#else
    return r.ru_maxrss; // Linux
#endif
}

// "data/array_input/1000_ascendente_D1_a.txt" -> ("1000","ascendente","D1","a")
static void parse_nombre(const std::string& path, std::string out[4]) {
    size_t slash = path.find_last_of("/\\");
    std::string base = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = base.rfind(".txt");
    if (dot != std::string::npos) base.resize(dot);

    for (int i = 0; i < 4; ++i) out[i].clear();
    int campo = 0;
    for (char c : base) {
        if (c == '_' && campo < 3) { ++campo; continue; }
        out[campo].push_back(c);
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "uso: %s <algoritmo> <entrada> <salida|->\n", argv[0]);
        return 1;
    }
    const std::string algo = argv[1];
    const std::string in_path = argv[2];
    const std::string out_path = argv[3];

    void (*sort_fn)(std::vector<int>&) = nullptr;
    if (algo == "mergesort")          sort_fn = merge_sort;
    else if (algo == "quicksort")     sort_fn = quick_sort;
    else if (algo == "patiencesort")  sort_fn = patience_sort;
    else if (algo == "stdsort")       sort_fn = std_sort;
    else {
        std::fprintf(stderr, "algoritmo desconocido: %s\n", algo.c_str());
        return 1;
    }

    // leer arreglo de entrada
    std::vector<int> original;
    {
        std::ifstream in(in_path);
        if (!in) {
            std::fprintf(stderr, "no puedo abrir %s\n", in_path.c_str());
            return 1;
        }
        int x;
        while (in >> x) original.push_back(x);
    }

    std::vector<int> work(original.begin(), original.end());
    const long base_kb = peak_rss_kb();

    // tiempo
    long reps = 0;
    double total_ms = 0.0;
    const auto wall0 = Clock::now();
    do {
        work.assign(original.begin(), original.end());
        const auto t0 = Clock::now();
        sort_fn(work);
        const auto t1 = Clock::now();
        total_ms += std::chrono::duration<double, std::milli>(t1 - t0).count();
        ++reps;
    } while (std::chrono::duration<double, std::milli>(Clock::now() - wall0).count() < 200.0
             && reps < 100000);

    const long peak_kb = peak_rss_kb();
    const double time_ms = total_ms / reps;
    long mem_kb = peak_kb - base_kb;
    if (mem_kb < 0) mem_kb = 0;

    if (!std::is_sorted(work.begin(), work.end())) {
        std::fprintf(stderr, "ADVERTENCIA: %s no dejo ordenado %s\n",
                     algo.c_str(), in_path.c_str());
    }

    // escribir arreglo ordenado
    if (out_path != "-") {
        std::ofstream out(out_path);
        for (size_t i = 0; i < work.size(); ++i)
            out << work[i] << (i + 1 < work.size() ? ' ' : '\n');
    }

    // linea CSV
    std::string f[4];
    parse_nombre(in_path, f);
    char host[256] = "desconocida";
    gethostname(host, sizeof(host));
    std::printf("%s,%s,%s,%s,%s,%.6f,%ld,%s\n",
                algo.c_str(), f[0].c_str(), f[1].c_str(), f[2].c_str(), f[3].c_str(),
                time_ms, mem_kb, host);
    return 0;
}
