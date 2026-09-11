"""
INF-221 TAREA 1 2026-2 — Algoritmos y Complejidad
Generador de graficos para los algoritmos de ordenamiento de arreglos.
Autor: Felipe Carrasco P, rol 202473522-5

Lee ../data/measurements/sorting.csv   (lo produce `make measurements`)
Escribe ../data/plots/*.png

Genera, por cada dominio (D1, D7):
  - sorting_tiempo_<dom>.png : tiempo vs n (log-log), un subplot por tipo de entrada, una curva por algoritmo, con guias teoricas n*log(n) y n^2.
  - sorting_memoria_<dom>.png: memoria auxiliar (pico RSS - base) vs n, con guia O(n).

Las 3 muestras (a, b, c) de cada configuracion se promedian.
"""

import os

import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
CSV = os.path.join(HERE, "..", "data", "measurements", "sorting.csv")
PLOT_DIR = os.path.join(HERE, "..", "data", "plots")

ALGOS = ["mergesort", "quicksort", "patiencesort", "stdsort"]
TIPOS = ["ascendente", "descendente", "aleatorio"]
DOMINIOS = ["D1", "D7"]
MARKER = {"mergesort": "o", "quicksort": "s", "patiencesort": "^", "stdsort": "D"}


def cargar():
    if not os.path.exists(CSV):
        raise SystemExit(f"no existe {CSV}\ncorre 'make measurements' primero")
    df = pd.read_csv(CSV)  # las filas con timeout traen tiempo/memoria = NA -> NaN

    # No mezclar mediciones de distintas maquinas en un mismo grafico.
    maquinas = sorted(df["maquina"].dropna().unique())
    if len(maquinas) > 1:
        sel = os.environ.get("MAQUINA")
        if sel not in maquinas:
            raise SystemExit(
                f"el CSV tiene datos de varias maquinas: {maquinas}\n"
                f"elegi una con:  MAQUINA=<nombre> make plots")
        df = df[df["maquina"] == sel]

    return (df.groupby(["algoritmo", "n", "tipo", "dominio"], as_index=False)
              [["tiempo_ms", "memoria_kb"]].mean())


def guia(ax, ns, ys_ref, func, etiqueta, estilo):
    """Dibuja c*func(n) escalada para pasar por el ultimo punto finito de ys_ref."""
    ns = np.asarray(ns, dtype=float)
    ys_ref = np.asarray(ys_ref, dtype=float)
    fin = np.isfinite(ys_ref)
    if not fin.any():
        return
    base = func(ns)
    c = ys_ref[fin][-1] / base[fin][-1]
    ax.plot(ns, c * base, estilo, color="gray", linewidth=1, label=etiqueta, zorder=1)


def figura(df, columna, titulo, ylabel, nombre, guias, yscale="log"):
    for dom in DOMINIOS:
        fig, axes = plt.subplots(1, len(TIPOS), figsize=(4.2 * len(TIPOS), 4.0),
                                 sharey=True)
        for ax, tipo in zip(axes, TIPOS):
            sub = df[(df.dominio == dom) & (df.tipo == tipo)]
            if sub.empty:
                ax.set_visible(False)
                continue
            ref_y = None
            for algo in ALGOS:
                s = sub[sub.algoritmo == algo].sort_values("n")
                if s.empty:
                    continue
                y = s[columna].to_numpy(dtype=float)
                if yscale == "log":
                    y = np.where(y <= 0, np.nan, y)
                ax.plot(s["n"], y, marker=MARKER[algo], label=algo)
                if algo == "mergesort":
                    ref_y = (s["n"].to_numpy(), s[columna].to_numpy(dtype=float))

            if ref_y is not None:
                ns, ys = ref_y
                for func, etiq, estilo in guias:
                    guia(ax, ns, ys, func, etiq, estilo)

            ax.set_xscale("log")
            ax.set_yscale(yscale, **({"linthresh": 1} if yscale == "symlog" else {}))
            ax.set_xlabel("n")
            ax.set_title(f"{tipo}")
            ax.grid(True, which="both", alpha=0.3)
        axes[0].set_ylabel(ylabel)
        fig.suptitle(f"{titulo} — dominio {dom}")
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc="lower center", ncol=len(labels),
                   bbox_to_anchor=(0.5, -0.02))
        fig.tight_layout(rect=(0, 0.05, 1, 0.96))
        out = os.path.join(PLOT_DIR, f"{nombre}_{dom}.png")
        fig.savefig(out, dpi=130, bbox_inches="tight")
        plt.close(fig)
        print(f"-> {out}")


def main():
    os.makedirs(PLOT_DIR, exist_ok=True)
    df = cargar()
    figura(df, "tiempo_ms", "Tiempo de ordenamiento", "tiempo [ms]", "sorting_tiempo",
           guias=[(lambda n: n * np.log2(n), "n log n", ":"),
                  (lambda n: n ** 2, "n^2", "--")])
    figura(df, "memoria_kb", "Memoria auxiliar", "memoria [kB]", "sorting_memoria",
           guias=[(lambda n: n, "O(n)", "--")], yscale="symlog")


if __name__ == "__main__":
    main()
