CPP=clang++-3.6 -std=c++11
CPPOPTS = -c -Wall -Wextra -O3 -I.
OBJS = tcp.o tcp_states.o
PROG = tcp_v2

all: $(OBJS)
	$(CPP) $(OBJS) main.cpp -o $(PROG)

%.o: %.cpp %.hpp
	$(CPP) -c  $< -o $@

%:%.cpp
	$(CPP) $< -o $@

clean:
	$(RM) $(OBJS) $(PROG)



