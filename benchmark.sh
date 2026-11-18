#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

BENCH_ROOT="examples/benchmarks"

# plem v bplem
#N=(2 4 8 16 32 64 128 256 512 1024)
#M=(4)
# bplem v large m
N=(512)
M=(4 8 16 32 64)
# bplem v large n
#N=(8192 16384 32768 65536 131072 262144 524288 1048576 2097152 4194304 8388608 16777216)
#M=(4)

DIR_SUFFIX="${DIR_SUFFIX:-}"
BIN="${BIN:-./main}"
ALGO="${ALGO:-BDPLEM}"
THREAD_LIST=(1 2 4 8)

RESULTS_CSV="${RESULTS_CSV:-results.csv}"

if [[ ! -f "$RESULTS_CSV" ]]; then
  echo "n,m,algo,threads,cpu_time,wall_time,intersection_profile" > "$RESULTS_CSV"
fi

# make -j

run_case() {
  local dir="$1"
  local in_file="$dir/problem.csv"
  local out_file="$dir/solution.csv"

  if [[ ! -d "$dir" ]]; then
    echo "Skipping (missing dir): $dir"
    return 0
  fi
  if [[ ! -f "$in_file" ]]; then
    echo "Skipping (missing input): $in_file"
    return 0
  fi

  echo -e "\nRunning: $in_file -> $out_file (ALGO=$ALGO THREADS=$THREADS)"
  output=$("$BIN" "$in_file" "$out_file" "$ALGO" "$THREADS")
  echo "$output"

  base=$(basename "$dir")
  n_val=${base%x*}
  m_val=${base#*x}

  cpu_time=$(echo "$output" | grep "CPU time" | awk '{print $3}' | sed 's/s//')
  wall_time=$(echo "$output" | grep "Wall time" | awk '{print $3}' | sed 's/s//')
  #intersection_profile=$(echo "$output" | grep -A 1 "^Intersection profile:" | tail -n 1 | tr -d '\r')

  if [[ -n "$cpu_time" && -n "$wall_time" ]]; then
    #echo "$n_val,$m_val,$ALGO,$THREADS,$cpu_time,$wall_time,\"$intersection_profile\"" >> "$RESULTS_CSV"
    echo "$n_val,$m_val,$ALGO,$THREADS,$cpu_time,$wall_time" >> "$RESULTS_CSV"
  else
    echo "Warning: Failed to parse metrics for $dir"
  fi
}

if [[ "$ALGO" == "BDPLEM" || "$ALGO" == "PDPLEM" ]]; then
  for threads in "${THREAD_LIST[@]}"; do
    THREADS="$threads"
    for n in "${N[@]}"; do
      for m in "${M[@]}"; do
        run_case "$BENCH_ROOT/${n}x${m}${DIR_SUFFIX}"
      done
    done
  done
else
  THREADS="${THREADS:-1}"
  for n in "${N[@]}"; do
    for m in "${M[@]}"; do
      run_case "$BENCH_ROOT/${n}x${m}${DIR_SUFFIX}"
    done
  done
fi

echo "Done."