


void State::active_open(TCP::Connection*) {
	// Default active open.
	// Nothing.
}

void State::passive_open(TCP::Connection*) {
	// Default passive open.
	// Nothing.
}

void State::synchronize(TCP::Connection*, Packet&) {
	// Default syn.
	// Drop.
}

void State::acknowledge(TCP::Connection*, Packet&) {
	// Default ack.
	// Drop.
}

void State::close(TCP::Connection*) {
	// Default close.
}

void State::reset(TCP::Connection*) {
	// Send RESET
}

void State::set_state(TCP::Connection* sock, State& state) {
	sock.set_state(state);
}

std::string State::to_string() const {
	return "STATELESS";
}


///////////////// CONCRETE STATES /////////////////
/*
	CLOSED
*/
static State& Closed::instance() {
	static State* instance = new Closed();
	return *instance;
}
void Closed::passive_open(TCP::Socket* sock) {
	set_state(sock, Listen::instance());
}

void Closed::active_open(TCP::Socket* sock) {
	// SEND SYN
	set_state(sock, SynSent::instance());
}

/*
	LISTEN
*/
static State& Listen::instance() {
	static State* instance = new Listen();
	return *instance;
}

void Listen::synchronize(TCP::Connection* conn) {
	if(!ACK) {
		// SEND SYN+ACK
		set_state(conn, SynReceived::instance());	
	} else {
		// Somethings wrong
		// RESET
	}
}

/*
	SYN-SENT
*/
static State& SynSent::instance() {
	static State* instance = new SynSent();
	return *instance;
}

void SynSent::synchronize(TCP::Connection* conn) {
	if(ACK) { // If ACK is present (SYN+ACK)
		if(ACK != ack) {
			// RESET
		}
		// SEND ACK
		set_state(conn, Established::instance());
	}
}

/*
	SYN-RCV
*/
static State& SynReceived::instance() {
	static State* instance = new SynReceived();
	return *instance;
}

void SynReceived::acknowledge(TCP::Connection* conn) {
	if(!RST) {
		set_state(conn, Established::instance());
	} else {
		set_state(conn, Listen::instance());
	}
	
}

/*
	ESTABLISHED
*/
static State& Established::instance() {
	static State* instance = new Established();
	return *instance;
}

void Established::acknowledge(TCP::Connection* conn, Packet& p) {
	//every data comes with ack right?
	conn->process(p);
}