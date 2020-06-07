CC = g++
objects-dtp = $(addprefix server-dtp/,main.o)
objects-pi = $(addprefix server-pi/,main.o)
exe = main.exe
exe-dtp = server-dtp/main.exe
exe-pi = server-pi/main.exe

default:
	@echo Please specify either make dtp or make pi 

dtp: $(objects-dtp)
	$(CC) -o $(exe-dtp) $(objects-dtp) && rm $(objects-dtp)

pi: $(objects-pi)
	$(CC) -o $(exe-pi) $(objects-pi) && rm $(objects-pi)


%.o: %.cpp
	$(CC) -c -o $@ $<

.PHONY: clean
clean:
	@rm server-dtp/*.o server-pi/*.o || :