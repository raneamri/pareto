import argparse
import random
import shutil
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

FILE: str = "fronts.csv"

NUM_FRONTS: int = 64
PIECES: int = 4
MAX_X: int = 64
MIN_X: int = 1
MAX_Y: int= 64
MIN_Y: int = 1

class ParetoFront:
    def __init__(self, m):
        self._pieces = m
        self._coords: list[tuple] = []

    def generate(self):
        self._coords = []

        m = self._pieces
        if m <= 0:
            return

        x_start = random.uniform(MIN_X, MAX_X/2)
        x_end = random.uniform(x_start, MAX_X)

        alpha = random.uniform(1.5, 2.0)

        y_scale = random.uniform(1.0, 15.0)

        for i in range(m):
            t = i / (m - 1) if m > 1 else 0.0

            x = x_start + t * (x_end - x_start)

            if i == 0:
                y = random.uniform(32.0, 64.0)
            else:
                y_prev = self._coords[-1][1]

                decrement = y_scale * ((1 - t) ** alpha) + random.uniform(0.01, 0.5)
                y = y_prev - decrement

            if x <= 0 or y <= 0: # out of range, try again
                self.generate()
                return

            self._coords.append((round(x, 6), round(y, 6)))

    def dump(self, file: str, format = 1):
        if format == 1:
            with open(file, "a") as f:
                for coord in self._coords:
                    f.write(f"({coord[0]}, {coord[1]})," if coord != self._coords[-1] else f"({coord[0]}, {coord[1]})")

                f.seek(f.tell() - 1, 0)
                f.write("\n")
        elif format == 0:
            with open(file, "a") as f:
                for coord in self._coords:
                    for point in coord:
                        f.write(f"{point}{"," if point != self._coords[-1][-1] else ""}")

                f.seek(f.tell() - 1, 0)
                f.write("\n")


def plot_all(fronts: list[ParetoFront], intervals: list[tuple] = []):
    for pf in fronts:
        x, y = zip(*pf._coords)
        plt.plot(x, y)
    if intervals:
        for interval in intervals:
            plt.axvline(x=interval, color='r', linestyle='--')
    plt.xlabel("X-axis")
    plt.ylabel("Y-axis")
    plt.title("All Pareto Fronts")
    plt.show()

def parse_int_list(raw: str | None, default: list[int]) -> list[int]:
    if not raw:
        return default[:]
    values = [token.strip() for token in raw.replace(",", " ").split() if token.strip()]
    return [int(token) for token in values]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dump", default="../examples/benchmarks")
    parser.add_argument("--n", dest="n_values", default="64")
    parser.add_argument("--m", dest="m_values", default="4,8,16,32,64")
    parser.add_argument("--seed", type=int, default=None)
    parser.add_argument("--clean", action="store_true")
    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)
        np.random.seed(args.seed)

    config = {
        "n": parse_int_list(args.n_values, [64]),
        "m": parse_int_list(args.m_values, [4, 8, 16, 32, 64]),
    }

    generate_benchmarks(Path(args.dump), config, clean=args.clean)


def generate_benchmarks(dump: Path, config: dict, clean: bool = False):
    dump.mkdir(parents=True, exist_ok=True)

    for n in config["n"]:
        for m in config["m"]:
            dataset_dir = dump / f"{n}x{m}"
            if clean and dataset_dir.exists():
                shutil.rmtree(dataset_dir)
            dataset_dir.mkdir(parents=True, exist_ok=True)

            file = dataset_dir / "problem.csv"
            with open(file, "w") as f:
                f.write("")

            for _ in range(n):
                pf = ParetoFront(m)
                pf.generate()
                pf.dump(str(file), 1)


if __name__ == "__main__":
    main()
