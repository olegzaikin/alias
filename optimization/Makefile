CPP = g++
CPPFLAGS = -O3 -std=c++0x

alias: sat_solver.o base_ls.o igbfs.o alias.o
	${CPP} ${CPPFLAGS} sat_solver.o base_ls.o igbfs.o alias.o -o alias

alias.o: alias.cpp
	${CPP} ${CPPFLAGS} alias.cpp -c

igbfs.o: igbfs.cpp
	${CPP} ${CPPFLAGS} igbfs.cpp -c

base_ls.o: base_ls.cpp
	${CPP} ${CPPFLAGS} base_ls.cpp -c

sat_solver.o: sat_solver.cpp
	${CPP} ${CPPFLAGS} sat_solver.cpp -c

clean:
	rm -rf *.o
	rm -f alias_ls
	clear
