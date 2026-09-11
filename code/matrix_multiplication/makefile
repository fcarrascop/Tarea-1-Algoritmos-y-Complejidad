# INF-221 TAREA 1 2026-2 — Multiplicacion de matrices
#
#   make            compila el binario ./matrix_multiplication
#   make inputs     genera los datasets en data/matrix_input/ (semilla fija)
#   make measurements   corre naive y strassen sobre cada par -> data/measurements/matrix_multiplication.csv
#   make plots      genera los graficos PNG en data/plots/
#   make clean      borra el binario

CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

ALGO_SRC := algorithms/naive.cpp algorithms/strassen.cpp
ALGO_HDR := algorithms/matmul.hpp
BIN      := matrix_multiplication

INPUT_DIR   := data/matrix_input
OUTPUT_DIR  := data/matrix_output
MEASURE_DIR := data/measurements
PLOT_DIR    := data/plots
CSV         := $(MEASURE_DIR)/matrix_multiplication.csv

ALGOS  := naive strassen
# un dataset por cada archivo _1.txt (su pareja es el _2.txt)
INPUTS := $(sort $(wildcard $(INPUT_DIR)/*_1.txt))

# Strassen (sin umbral de corte) es lento pero medible: n=1024 densa tarda
# ~180 s y ~50 MB por corrida (las 18 corridas de n=1024 ~54 min, se lanzan
# de noche). Se lo omite solo por encima de este n.
MAX_STRASSEN_N := 1024

# Limite de tiempo por medicion (ver comentario en code/sorting/makefile).
# Al superarlo se escribe una fila con tiempo/memoria = NA y se sigue.
TIMEOUT_SECS ?= 600
TIMEOUT      := python3 scripts/run_timeout.py $(TIMEOUT_SECS)

.PHONY: all inputs measurements plots clean

all: $(BIN)

$(BIN): matrix_multiplication.cpp $(ALGO_SRC) $(ALGO_HDR)
	$(CXX) $(CXXFLAGS) -o $@ matrix_multiplication.cpp $(ALGO_SRC)

inputs:
	cd scripts && python3 matrix_generator.py

measurements: $(BIN)
	@mkdir -p $(OUTPUT_DIR) $(MEASURE_DIR)
	@echo "algoritmo,n,tipo,dominio,muestra,tiempo_ms,memoria_kb,maquina" > $(CSV)
	@maquina=$$(hostname); \
	for f1 in $(INPUTS); do \
		f2=$${f1%_1.txt}_2.txt; \
		base=$$(basename $${f1%_1.txt}); \
		n=$$(echo $$base | cut -d_ -f1); t=$$(echo $$base | cut -d_ -f2); \
		d=$$(echo $$base | cut -d_ -f3); m=$$(echo $$base | cut -d_ -f4); \
		for a in $(ALGOS); do \
			if [ "$$a" = "strassen" ] && [ "$$n" -gt $(MAX_STRASSEN_N) ]; then \
				echo "  omitido: strassen $$base (n>$(MAX_STRASSEN_N))"; continue; fi; \
			if [ "$$a" = "naive" ]; then out=$(OUTPUT_DIR)/$${base}_out.txt; else out=-; fi; \
			row=$$($(TIMEOUT) ./$(BIN) $$a $$f1 $$f2 $$out); st=$$?; \
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
