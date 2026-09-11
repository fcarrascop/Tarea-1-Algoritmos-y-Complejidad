# INF-221 TAREA 1 2026-2 — Ordenamiento
#
#   make            compila el binario ./sorting
#   make inputs     genera los datasets en data/array_input/ (semilla fija)
#   make measurements   corre los 4 algoritmos sobre cada dataset -> data/measurements/sorting.csv
#   make plots      genera los graficos PNG en data/plots/
#   make clean      borra el binario

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

ALGO_SRC := algorithms/mergesort.cpp algorithms/quicksort.cpp \
            algorithms/patiencesort.cpp algorithms/sort.cpp
ALGO_HDR := algorithms/algorithms.hpp
BIN      := sorting

INPUT_DIR   := data/array_input
OUTPUT_DIR  := data/array_output
MEASURE_DIR := data/measurements
PLOT_DIR    := data/plots
CSV         := $(MEASURE_DIR)/sorting.csv

ALGOS  := mergesort quicksort patiencesort stdsort
INPUTS := $(sort $(wildcard $(INPUT_DIR)/*.txt))

# Limite de tiempo por medicion. QuickSort con pivote fijo es O(n^2) en
# entradas ordenadas y tambien en D1 (dominio chico -> muchos duplicados):
# a n=10^7 puede tardar horas. Al superar el limite se escribe una fila con
# tiempo/memoria = NA y se sigue. Wrapper portable en Python (Mac y Linux).
TIMEOUT_SECS ?= 300
TIMEOUT      := python3 scripts/run_timeout.py $(TIMEOUT_SECS)

.PHONY: all inputs measurements plots clean

all: $(BIN)

$(BIN): sorting.cpp $(ALGO_SRC) $(ALGO_HDR)
	$(CXX) $(CXXFLAGS) -o $@ sorting.cpp $(ALGO_SRC)

inputs:
	cd scripts && python3 array_generator.py

measurements: $(BIN)
	@mkdir -p $(OUTPUT_DIR) $(MEASURE_DIR)
	@echo "algoritmo,n,tipo,dominio,muestra,tiempo_ms,memoria_kb,maquina" > $(CSV)
	@maquina=$$(hostname); \
	for f in $(INPUTS); do \
		base=$$(basename $$f .txt); \
		n=$$(echo $$base | cut -d_ -f1); t=$$(echo $$base | cut -d_ -f2); \
		d=$$(echo $$base | cut -d_ -f3); m=$$(echo $$base | cut -d_ -f4); \
		for a in $(ALGOS); do \
			if [ "$$a" = "stdsort" ]; then out=$(OUTPUT_DIR)/$${base}_out.txt; else out=-; fi; \
			row=$$($(TIMEOUT) ./$(BIN) $$a $$f $$out); st=$$?; \
			if [ $$st -eq 0 ]; then \
				echo "$$row" >> $(CSV); \
			else \
				echo "$$a,$$n,$$t,$$d,$$m,NA,NA,$$maquina" >> $(CSV); \
				echo "  timeout/error ($$st): $$a $$base"; \
			fi; \
		done; \
		echo "  medido: $$base"; \
	done
	@echo "-> $(CSV)"

plots:
	@mkdir -p $(PLOT_DIR)
	cd scripts && python3 plot_generator.py

clean:
	rm -f $(BIN)
