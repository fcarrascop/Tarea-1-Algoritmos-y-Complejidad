"""
INF-221 TAREA 1 2026-2 — Algoritmos y Complejidad
Autor: Felipe Carrasco P, rol 202473522-5

Ejecuta un comando con limite de tiempo, de forma portable (Mac y Linux) sin depender de coreutils. Uso:

    python3 run_timeout.py <segundos> <comando> [args...]

Devuelve el codigo de salida del comando, o 124 si se agoto el tiempo (misma convencion que GNU `timeout`). La salida estandar del comando pasa sin cambios.
"""

import subprocess
import sys

if len(sys.argv) < 3:
    sys.exit("uso: run_timeout.py <segundos> <comando> [args...]")

secs = float(sys.argv[1])
try:
    sys.exit(subprocess.run(sys.argv[2:], timeout=secs).returncode)
except subprocess.TimeoutExpired:
    sys.exit(124)
