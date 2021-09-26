all : decode
decode : decode.c
	gcc decode.c -o decode
