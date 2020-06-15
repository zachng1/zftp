CC = g++
compile-flags = -g -Wall -Wextra -pedantic -std=c++17
link-flags = -pthread

objects-dtp = $(addprefix server-dtp/,main.o)
objects-pi = $(addprefix server-pi/,listener.o user.o handlerfunctions.o main.o)
objects-launcher = launcher.cpp
exe-dtp = server-dtp/dtp
exe-pi = server-pi/pi
exe-launcher = launcher

$(exe-launcher): $(exe-pi) $(exe-dtp)
	$(CC) $(compile-flags) -o $(exe-launcher) $(objects-launcher) $(link-flags)

$(exe-pi): $(objects-pi)
	$(CC) $(compile-flags) -o $(exe-pi) $(objects-pi) $(link-flags)
$(exe-dtp): $(objects-dtp)
	$(CC) $(compile-flags) -o $(exe-dtp) $(objects-dtp) $(link-flags)	
	
%.o: %.cpp
	$(CC) $(compile-flags) -c -o $@ $< $(link-flags)

.PHONY: clean pi dtp launcher
clean:
	@rm server-dtp/*.o server-pi/*.o || :
pi: $(objects-pi)
	$(CC) $(compile-flags) -o $(exe-pi) $(objects-pi) $(link-flags)
dtp: $(objects-dtp)
	$(CC) $(compile-flags) -o $(exe-dtp) $(objects-dtp) $(link-flags)
