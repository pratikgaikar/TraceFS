default:
	make without_extra_credit

with_extra_credit:
	gcc -Wall treplay.c -lz -o treplay -D EXTRA_CREDIT=10
	gcc -Wall trctl.c -o trctl -D EXTRA_CREDIT=10
	gcc read.c -o read
	gcc write.c -o write

without_extra_credit:
	gcc treplay.c -Wall -o treplay
	gcc trctl.c -Wall -o trctl
	gcc read.c -o read
	gcc write.c -o write

