hello:
	gcc -o main main.c vector.c barbershop.c -lpthread -fsanitize=address
	./main