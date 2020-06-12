CC = g++
compile-flags = -Wall -Wextra -pedantic -std=c++17
link-flags = -pthread

objects-dtp = $(addprefix server-dtp/,main.o)
objects-pi = $(addprefix server-pi/,main.o listener.o user.o)
exe = main
exe-dtp = server-dtp/dtp
exe-pi = server-pi/pi

default:
	@echo Please specify either make dtp or make pi 

$(exe-dtp): $(objects-dtp)
	
%.o: %.cpp
	$(CC) $(compile-flags) -c -o $@ $< $(link-flags)

.PHONY: clean pi dtp
clean:
	@rm server-dtp/*.o server-pi/*.o || :
pi: $(objects-pi)
	$(CC) $(compile-flags) -o $(exe-pi) $(objects-pi) $(link-flags)
dtp: $(objects-dtp)
	$(CC) $(compile-flags) -o $(exe-dtp) $(objects-dtp) $(link-flags)