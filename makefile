all: mygit

clean:
	rm mygit main.o functions.o repository.o commit.o staging_area.o

mygit: main.o functions.o repository.o commit.o staging_area.o
	gcc main.o functions.o repository.o commit.o staging_area.o -o mygit -lcrypto

main.o: main.c functions.h repository.h
	gcc -c main.c -o main.o

repository.o: repository.c repository.h commit.h staging_area.h
	gcc -c repository.c -o repository.o

commit.o: commit.c commit.h repository.h
	gcc -c commit.c -o commit.o

functions.o: functions.c functions.h repository.h commit.h staging_area.h
	gcc -c functions.c -o functions.o

staging_area.o: staging_area.c staging_area.h
	gcc -c staging_area.c -o staging_area.o