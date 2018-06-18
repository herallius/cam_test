all:
	gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror cam_test.c -o cam_test
	