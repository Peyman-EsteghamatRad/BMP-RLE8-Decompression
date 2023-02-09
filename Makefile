all: main
main: supporting_program.c V0.c bitmap.c V1.c
	echo "Building executable!"
	gcc -w -O2 -o $@ $^

clean:
	rm -f main
	rm -f bin/*
	rm -f correctnessTests/results/*
