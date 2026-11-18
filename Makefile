build:
	gcc -Wall -Xpreprocessor -fopenmp -O2 -I/opt/homebrew/opt/libomp/include \
		-L/opt/homebrew/opt/libomp/lib -lomp \
		main.c util.c dplem.c bdplem.c pdplem.c -o main

hcl_build:
	gcc -fopenmp -fopenacc -O2 main.c util.c dplem.c bdplem.c pdplem.c -o main -lm

hcl_run:
	make hcl_build && make pure_run

run:
	make build && make pure_run

pure_run:
	./main ./problem.csv ./solution.csv BPLEM 8
	
data:
	uv run main.py --n "16777216" --m "4" --seed 42 --clean

#lldb ./main
