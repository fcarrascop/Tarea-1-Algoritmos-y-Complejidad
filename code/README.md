# INF-221 Tarea 1 (2026-2) — Análisis experimental de algoritmos

Autor: `Felipe Carrasco P` — rol `202473522-5`

Estudio experimental de 4 algoritmos de ordenamiento y 2 de multiplicación de matrices: se mide tiempo y memoria en la práctica y se contrasta con la complejidad teórica.

## Requisitos

- `g++` con soporte C++17 (probado con g++ de Ubuntu y Apple Clang en macOS)
- `make`
- Python 3 con `numpy`, `pandas`, `matplotlib`

```bash
pip3 install numpy pandas matplotlib
```

## Uso

Los cuatro pasos, en orden, para cada problema. Ejecutar dentro de `code/sorting/` o `code/matrix_multiplication/` según corresponda.

```bash
make # 1. compila el binario de mediciones
make inputs # 2. genera los datasets en data/*_input/
make measurements # 3. corre todos los algoritmos sobre todos los datasets -> data/measurements/*.csv
make plots # 4. genera los gráficos PNG en data/plots/
make clean # borra el binario
```

`make measurements` con el set completo es lento. Cada medición tiene un límite de tiempo (`scripts/run_timeout.py`, portable Mac/Linux); al superarlo se escribe una fila con `tiempo_ms = NA` y se continúa. Ajustable:

```bash
make measurements TIMEOUT_SECS=120 # límite por medición (default: 300 sorting / 600 matrices)
make plots MAQUINA=<hostname> # si el CSV mezcla varias máquinas, elegir una
```

Para matrices, Strassen se omite para `n > 1024` (variable `MAX_STRASSEN_N`).

### Ejecución individual

```bash
# ordenamiento
./sorting <mergesort|quicksort|patiencesort|stdsort> <entrada.txt> <salida.txt|->

# matrices
./matrix_multiplication <naive|strassen> <M1.txt> <M2.txt> <salida.txt|->
```

Ambos imprimen a stdout una línea CSV: `algoritmo,n,tipo,dominio,muestra,tiempo_ms,memoria_kb,maquina`
(`maquina` = `gethostname()`, para no mezclar mediciones de equipos distintos). Con `-` como archivo de salida no escriben el resultado (solo miden). `matrix_multiplication` además verifica `strassen == naive` en cada corrida de Strassen (falla con código 2 si no coinciden).

## Formato de archivos

### Ordenamiento — `{n}_{t}_{d}_{m}.txt`

Una línea con los `n` enteros separados por espacio.

| campo | valores |
|-------|---------|
| `n` | `10`, `1000`, `100000`, `10000000` ($10^1$, $10^3$, $10^5$, $10^7$) |
| `t` | `ascendente`, `descendente`, `aleatorio` |
| `d` | `D1` (dominio `{0..9}`), `D7` (dominio `{0..10^7}`) |
| `m` | `a`, `b`, `c` (muestra aleatoria) |

Salida `{n}_{t}_{d}_{m}_out.txt`: el arreglo ordenado, mismo formato.

### Multiplicación de matrices — `{n}_{t}_{d}_{m}_1.txt` y `_2.txt`

`n` líneas de `n` enteros separados por espacio.

| campo | valores |
|-------|---------|
| `n` | `16`, `64`, `256`, `1024` ($2^4$, $2^6$, $2^8$, $2^{10}$) |
| `t` | `densa`, `diagonal`, `dispersa` |
| `d` | `D0` (dominio `{0,1}`), `D10` (dominio `{0..9}`) |
| `m` | `a`, `b`, `c` |

Salida `{n}_{t}_{d}_{m}_out.txt`: el producto `M1 × M2`, `n` líneas de `n` enteros.

## Metodología de medición

- **Un proceso por medición.** El makefile invoca el binario una vez por cada combinación (algoritmo, n, tipo, dominio, muestra), para que el pico de memoria y el tiempo de un algoritmo no contaminen al siguiente.
- **Tiempo:** `std::chrono::steady_clock` alrededor de la llamada al algoritmo únicamente (sin la lectura/escritura de archivos). Para entradas chicas la corrida se repite hasta acumular ~200 ms y se promedia.
- **Memoria:** pico de RSS del proceso (`getrusage`, `ru_maxrss`) menos una línea base (puesto que el computador ya ocupa memoria, igualmente lo ideal sería trabajar sin procesos de fondo, para no contaminar la medición). En sorting la línea base se toma con la copia de trabajo ya materializada (construida, no solo `reserve()` — las páginas no quedan residentes hasta escribirse), así la cifra es solo la memoria auxiliar del algoritmo: 0 para los in-place (quick, std::sort), ~n para merge y patience. En matrices incluye además la matriz resultado. Valores bajo la granularidad del RSS se reportan como 0.
- **Límite de tiempo:** cada medición corre bajo `scripts/run_timeout.py`; si se agota, la fila queda con `NA` y la corrida sigue. QuickSort (aun con pivote aleatorio) es O($n^2$) en dominio D1 por los duplicados, y a $n=10^7$ no termina en tiempo razonable, por lo tanto el `NA` es un resultado válido.

## Reproducibilidad

Los datasets **no se versionan** (los de $10^7$ / $1024$ son demasiado grandes). Se regeneran con `make inputs`. Los generadores usan semilla fija (`221`), de modo que producen archivos byte-idénticos en cualquier máquina con la misma versión de numpy.

## Decisiones de implementación

- **Patience sort:** la versión de GeeksForGeeks es O($n^2$) (mezcla de pilas por escaneo lineal). Se reemplazó por la de Rosetta Code, O($n \log n$), con `std::lower_bound` para el reparto y un min-heap para la mezcla. Se usa `std::vector` como contenedor de pila en lugar de `std::stack` (≈10× más rápido; ver comentario en el archivo).
- **Quick sort:** pivote **aleatorio** (randomized quicksort, CLRS cap. 7): índice uniforme en `[low, high]`, llevado al final, luego partición Lomuto de la fuente. Tiempo esperado O(n log n) para cualquier entrada. El RNG se siembra con semilla fija (221) al inicio de cada ordenamiento → mediciones reproducibles. La recursión va solo sobre la partición menor → pila O(log n).
  Nota: el pivote aleatorio elimina el peor caso O(n²) de las entradas ordenadas, pero **no** el de D1: con dominio `{0..9}` la altísima repetición degrada la partición de Lomuto a O(n²) sin importar el pivote (haría falta partición de 3 vías).
- **Strassen:** se conserva tal cual la fuente, con matrices `vector<vector<int>>` copiadas en cada nivel. El costo de asignación lo hace mucho más lento que naïve en todo el rango medible (hallazgo del informe, no un defecto a corregir). El makefile lo omite solo para `n > MAX_STRASSEN_N` (1024). `matrix_multiplication.cpp` verifica `strassen == naive` en cada corrida.
- **`strassen.cpp`:** `nextPowerOfTwo` se cambió a una versión entera con desplazamiento de bits (la original usaba `pow`/`log2`, con riesgo de redondeo).

## Fuentes

Cada archivo `.cpp` cita su fuente en la cabecera. Resumen:

- Merge Sort, Quick Sort (partición Lomuto), multiplicación naïve, Strassen: GeeksForGeeks
- Quick Sort — pivote aleatorio: CLRS, *Introduction to Algorithms*, cap. 7 (Randomized-Quicksort)
- Patience Sort: Rosetta Code, "Sorting algorithms/Patience sort" (GFDL 1.2 / CC BY-SA)
- `std::sort`: biblioteca estándar de C++ (`<algorithm>`)
