CC=gcc

all:
	${CC} common.c button.c controller.c  gpio.c  piezo.c adt.c -o controller -lpthread
