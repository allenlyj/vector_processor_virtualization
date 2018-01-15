CFLAGS:=-O3 -std=c99
SOURCE:=vp_control.c random_exp.c main.c
HEADER:=vp_control.h random_exp.h
.PHONY: clean

virt_sim: ${SOURCE} ${HEADER}
	gcc ${CFLAGS} ${SOURCE} -o virt_sim
clean:
	rm virt_sim
