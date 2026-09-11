#ifndef SORTING_ALGORITHMS_HPP
#define SORTING_ALGORITHMS_HPP

#include <vector>

// Interfaz comun para los 4 algoritmos de ordenamiento de la Tarea 1.
void merge_sort(std::vector<int>& arr);
void quick_sort(std::vector<int>& arr);
void patience_sort(std::vector<int>& arr);
void std_sort(std::vector<int>& arr);

#endif
