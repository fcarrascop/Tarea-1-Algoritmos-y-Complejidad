/*
 * INF-221 Tarea 1 2026-2 - Felipe Carrasco P, rol 202473522-5
 * Source: https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/
 *
 * Modificaciones respecto a la fuente:
 * 1. Pivote ALEATORIO: en cada llamada a partition se elige un indice uniforme en [low, high], se lleva al final y luego se aplica la particion de Lomuto de la fuente. Es el "randomized quicksort" clasico (CLRS, cap. 7): tiempo esperado O(n log n) para cualquier entrada. El generador se siembra con una semilla fija al inicio de cada ordenamiento, para que las mediciones sean reproducibles.
 * 2. La recursion va solo sobre la particion mas pequeña y se itera sobre la mayor: acota la profundidad de la pila a O(log n) y evita el stack overflow con n = 10^7.
 */

#include <random>
#include <vector>
#include "algorithms.hpp"
using namespace std;

namespace {
    // Semilla fija -> el pivote aleatorio da la misma secuencia en cada corrida y en cualquier maquina (mediciones reproducibles).
    constexpr unsigned QSORT_SEED = 221;
    std::mt19937 rng(QSORT_SEED);
}

int partition(vector<int>& arr, int low, int high) {

    // pivote aleatorio: indice uniforme en [low, high], llevado al final.
    // El sesgo de 'rng() % k' es despreciable para los k de esta tarea.
    int p = low + (int)(rng() % (unsigned)(high - low + 1));
    swap(arr[p], arr[high]);

    // choose the pivot
    int pivot = arr[high];

    // undex of smaller element and indicates
    // the right position of pivot found so far
    int i = low - 1;

    // Traverse arr[low..high] and move all smaller
    // elements on left side. Elements from low to
    // i are smaller after every iteration
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }

    // move pivot after smaller elements and
    // return its position
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// the QuickSort function implementation
void quickSort(vector<int>& arr, int low, int high) {

    while (low < high) {

        // pi is the partition return index of pivot
        int pi = partition(arr, low, high);

        // recursion sobre la particion mas chica, iteracion sobre la grande
        if (pi - low < high - pi) {
            quickSort(arr, low, pi - 1);
            low = pi + 1;
        } else {
            quickSort(arr, pi + 1, high);
            high = pi - 1;
        }
    }
}

void quick_sort(vector<int>& arr) {
    rng.seed(QSORT_SEED); // mismo pivote en cada corrida
    if (!arr.empty())
        quickSort(arr, 0, (int)arr.size() - 1);
}
