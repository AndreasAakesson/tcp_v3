class TCP {
	
	/*
		A Socket defining a host.
	*/
	class Socket {
	private:
		using IPAddress = std::string;
		using Port = uint16_t;		
	public:
		/*
			A zero initiliazed Socket
		*/
		Socket();
		/*
			A Socket initiliazed with Address and Port.
		*/
		Socket(IPAddress address, Port port);
		/* 
			Return THE unintialized socket.
		*/
		static const Socket& empty();
		/*
			Return the Socket as a string.
		*/
		inline std::string to_string();

	private:
		/* Data Members */
		IPAddress address_;
		Port port_;
		static const Socket uninitialized_socket = {"0.0.0.0.0", 0};
	}; // < class Socket

	/// IMPLEMENTATION ///
	Socket::Socket() {
		return uninitilized_socket;
	}

	Socket::Socket(IPAddress address, Port port) 
		: address_(address), port_(port) {
	}

	static const Socket& Socket::empty() {
		return &uninitilized_socket;
	}

	std::string to_string() const {
		return address_ + ":" + std::to_string(port_);
	}


	/*
		A Connection between two Sockets.
	*/
	class Connection {
	private:
		struct TCB {

		} tcb;
	public:
		using SuccessCallback = std::function<void(TCP::Connection conn)>;
		using ErrorCallback = std::function<void(TCP::ConnectionException err)>;

		struct Touple {
			//IPAddress local_address;
			Port local_port;
			IPAddress remote_adress;
			Port remote_port;

			bool operator==(const Touple &rhs) const {
		        return local_port == rhs.local_port 
		        		&& remote_address == rhs.remote_address
		        		&& remote_port == rhs.remote_port;
		    }

		    bool operator<(const Touple &rhs) const {
		        return local_port < rhs.local_port 
		        		|| (local_port == rhs.local_port && remote_address < rhs.remote_address)
		        		|| (local_port == rhs.local_port && remote_address == rhs.remote_address && remote_port < rhs.remote_port);
		    }
		};

		class State;

		class Exception : public std::exception {
		public:
			inline Exception(const char* error) : std::exception(error) {};
		};
		/*
			A connection without remote.
		*/
		Connection(IPStack& stack, Socket& source);

		/*
			A connection with remote.
		*/
		Connection(IPStack& stack, Socket& source, Socket& dest, int init_seq);

		/*
			Callback for accept()
		*/
		void onAccept(SuccessCallback callback);

		/*
			Callback for close()
		*/
		void onClose(SuccessCallback callback);

		/*
			Callback for errors.
		*/
		void onError(ErrorCallback callback);

		/*
			Copy Constructor. Allocate new buffers, I guess?
			Should be called before setting remote host on the new Connection;
			- When the connection are leaving LISTENING.
		*/
		Connection(const Connection& obj);

		/*
			Read content from Connection (Stream)
		*/
		std::string read(uint16_t buffer_size);

		/*
			Write content to Connection.
		*/
		void write(std::string data);

		int handle(Packet& packet);
		
		/*
			Gracefully close a connection.
		*/
		void close();

		////// TCP Logic //////
		/*
			SYN
		*/
		void synchronize(Packet& packet);

		/*
			ACK
		*/
		void acknowledge(Packet& packet);

		std::string to_string() const;

		~Connection();

		bool is_listening();
		bool is_established();
		bool is_connected();

		bool operator<(const Connection& rhs) const;

	private:
		Socket& src_;
		Socket& dest_;

		// Connection State - Listening/Established etc.
		State* state_;

		buffer* send_;
		buffer* receive_;
		buffer* retransmit_queue_;
		segment* current;

		uint32_t seq_;
		uint32_t ack_;

		SuccessCallback on_accept_;
		SuccessCallback on_close_;
		ErrorCallback on_error_;
		
		/*
			Helper method to check for the current state.
		*/
		Connection::is_state(std::string state);

	}; // < class Connection

	/// IMPLEMENTATION ///
	Connection::Connection(Socket& source) 
		: state_(TCP::Connection::Closed::instance()) {
			src_ = &source;
			dest_ = Socket();

			state_->passive_open(this);
	}

	Connection::Connection(Socket& source, Socket& dest) 
		: state_(TCP::state::Closed::instance()) {

		// Maybe edit to state::Closed (fictional), and let synchronize set state.
		state_->active_open(this, dest);
	}

	void Connection::write(std::string data) {
		// We got some important writing to do.
		// Handeled by state? (Avoids writing on connection who isn't ESTABLISHED).
	}

	void Connection::close() {
		// Make necessary FIN stuff.
		// Clean up resources.
	}

	std::string Connection::to_string() const {
		std::stringstream ss;
		ss << *state;
	}

	Connection::~Connection() {
		close();
	}

	// State bool checks
	bool Connection::is_state(std::string state) {
		return state_->to_string() == state;
	}

	bool Connection::is_listening() {
		return is_state("LISTENING");
	}

	bool Connection::is_established() {
		return is_state("ESTABLISHED");
	}

	bool Connection::is_connected() {
		return is_established();
	}

	bool Connection::operator<(const Connection& rhs) const {

	}

	void Connection::synchronize(Packet& packet) {
		state_->synchronize(this, packet);
	}

	void Connection::acknowledge(Packet& packet) {
		state_->acknowledge(this, packet);
	}

	void Connection::transmit(Packet& packet) {
		state_->transmit(this, packet);
	}

	int Connection::handle(Packet& packet) {
		
		/*
			TODO: Maybe need some nested FLAG-checking.
			Missing RST.
		*/
		switch(FLAG): {
			case SYN: {
				state->synchronize(this, packet);
			}
			break;
			case: FIN: {
				state->close(this);
			}
			break;
			case ACK: {
				state->acknowledge(this, packet);
			}
			break;
		}
		transmit(packet);
	}
	/////////////////////////////////////////////

	std::map<Port, Socket> listeners_;
	using Connection_Id = std::pair<Socket&, Socket&>;
	std::map<Connection_Id, Connection> connections_;
	using ConnectionCallback = Connection::SuccessCallback;
	using ConnectionErrorCallback = Connection::ErrorCallback;
	/*
		v2. What do we acctaully need. Identifier for connection:
		[Source IP]		KNOWN (We got this from tcp(local_ip_stack))
		[Source PORT]	UNKNOWN
		[Remote IP]		UNKNOWN
		[Remote PORT]	UNKNOWN
	*/

	/*
		Create an connection with two peers.
	*/
	inline Connection& create_connection(Socket& src, Socket& dest) {
		auto conn = connections_.emplace(std::make_pair(src, dest), {stack_, src, dest});
		return conn.second;
	};

	/*
		Create an passive connection with only local.
	*/
	inline Connection& create_connection(Socket& src) {
		auto conn = connections_.emplace(std::make_pair(src, Socket::empty()), {ip_stack_, src});
		return conn.second;
	};

	/*
		Retreive a free initial seq number.
	*/
	inline uint32_t get_initial_seq_number() {
		return 1;
	}
	


public:
	/*
		Allocate a listening connection.
	*/
	inline Connection& bind(Port port) {
		// create socket and register on port
		//listeners_.emplace(std::pair(port, {ip_stack.address, port}));
		return create_connection(ip_stack_, {ip_stack_.address, port});
	}

	/*
		Passive opening a listening connection with a callback.
	*/
	inline void open(Port port, ConnectionCallback cb) {
		bind(port).onAccept(cb);
	}

	/*
		Allocate more than one listening connection.
	*/
	inline Connection& bind(Port port, int connections) {
		// NVM, for this we need multimap. Future thing.
	}

	/*
		Make an active open connection.
	*/
	inline Connection& connect(Socket& remote_address) {
		// create src socket with random port and source ip
		return create_connection({ip_stack.address, random_port()}, remote_address);
		// create dest socket
		// bind sockets to connection.
		// bind callback?

		// create entry in TCB???

		return conn;
	}

	/*
		Make an active open connection with a callback.
	*/
	inline void connect(Socket& remote_address, ConnectionCallback callback) {
		connect(Socket&).onAccept(callback);
	}

	inline std::string status() const {
		sstream ss;
		ss << "HEADER\t\t\t\n";
		for(auto conn : connections_) {
			ss << conn << "\n";
		}
	}

	inline void close(Port port) {
		// Close all connections on the given port.
	}

	inline void close(Socket& socket) {
		// Close all connections for the local socket.
	}

	inline void close() {
		// Close every TCP connection.
	}

	inline int receive(Packet& packet) {
		// Check if connection exists.
		// If there is one listening -> copy and add new listening connection.
		// Set remote destination on the listening one.
		// change state with state machine.

		auto touple = get_touple(packet);
		// is there a connection?
		auto conn_it = connections_.find(touple);
		if(conn_it != connections_.end())
		{
			Connection& conn = conn_it->second;
			return conn.handle(packet);
		} 
		else 
		{
			// is there a listener?
			auto list_it = listeners_.find(touple.port);
			// yes
			if(list_it != listeners_.end()) {
				// copy connection
				Connection conn{list_it->second};
				connections_.emplace(touple, conn);
				return conn.handle(packet);
			}
		}
		// drop package.
		return 0;

	}

	inline Connection& find_connection(Socket& source, Socket& remote) {
		
	}

	
}