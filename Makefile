CC=gcc

all:
	${CC} button.c controller.c  gpio.c  piezo.c -o controller
