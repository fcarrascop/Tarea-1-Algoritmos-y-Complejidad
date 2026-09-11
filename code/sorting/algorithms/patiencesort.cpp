/*
 * INF-221 Tarea 1 2026-2 - Felipe Carrasco P, rol 202473522-5
 *
 * Source: https://rosettacode.org/wiki/Sorting_algorithms/Patience_sort
 *
 * Modificacion respecto a la fuente: se usa std::vector como contenedor de cada pila (tope = back()) en lugar de std::stack, puesto que este se apoya en std::deque, cuyo costo de asignacion por pila degrada el tiempo ~10x en la practica.
 */

#include <vector>
#include <iterator>
#include <algorithm>
#include <cassert>
#include "algorithms.hpp"

namespace {

    template <class E>
    struct pile_less {
        bool operator()(const std::vector<E>& pile1, const std::vector<E>& pile2) const {
            return pile1.back() < pile2.back();
        }
    };

    template <class E>
    struct pile_greater {
        bool operator()(const std::vector<E>& pile1, const std::vector<E>& pile2) const {
            return pile1.back() > pile2.back();
        }
    };

    template <class Iterator>
    void patience_sort(Iterator first, Iterator last) {
        typedef typename std::iterator_traits<Iterator>::value_type E;
        typedef std::vector<E> Pile;

        std::vector<Pile> piles;
        // sort into piles
        for (Iterator it = first; it != last; it++) {
            E& x = *it;
            Pile newPile;
            newPile.push_back(x);
            typename std::vector<Pile>::iterator i =
                std::lower_bound(piles.begin(), piles.end(), newPile, pile_less<E>());
            if (i != piles.end())
                i->push_back(x);
            else
                piles.push_back(std::move(newPile));
        }

        // priority queue allows us to merge piles efficiently
        // we use a min-heap (greater-than comparator) on the pile tops
        std::make_heap(piles.begin(), piles.end(), pile_greater<E>());
        for (Iterator it = first; it != last; it++) {
            std::pop_heap(piles.begin(), piles.end(), pile_greater<E>());
            Pile& smallPile = piles.back();
            *it = smallPile.back();
            smallPile.pop_back();
            if (smallPile.empty())
                piles.pop_back();
            else
                std::push_heap(piles.begin(), piles.end(), pile_greater<E>());
        }
        assert(piles.empty());
    }
}

void patience_sort(std::vector<int>& arr) {
    patience_sort(arr.begin(), arr.end());
}
