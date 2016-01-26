#include "tcp.hpp"

int main(int argc, char *argv[]) {
	TCP::ServerSocket sock;

	sock.open();
	sock.close();
	sock.close();
	sock.open();
	sock.close();

	return 0;
};