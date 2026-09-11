#ifndef MATRIX_MULTIPLICATION_ALGORITHMS_HPP
#define MATRIX_MULTIPLICATION_ALGORITHMS_HPP

#include <vector>

// Matriz cuadrada de enteros (fila-mayor).
using Matrix = std::vector<std::vector<int>>;

// Interfaz comun de los 2 algoritmos de multiplicacion de la Tarea 1.
Matrix naive_multiply(const Matrix& a, const Matrix& b);
Matrix strassen_multiply(const Matrix& a, const Matrix& b);

#endif
