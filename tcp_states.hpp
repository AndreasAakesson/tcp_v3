#ifndef TCP_STATES_HPP
#define TCP_STATES_HPP

/*
	TCP::Connection::State

	Interface for a TCP State.
*/
class State {
public:
	virtual void passive_open(TCP::Connection*);
	virtual void active_open(TCP::Connection*);
	virtual void synchronize(TCP::Connection*, Packet&);
	virtual void acknowledge(TCP::Connection*, Packet&);
	virtual void close(TCP::Connection*);
	virtual void reset(TCP::Connection*);
	/*// Do we need this?
	virtual void send(TCP::Connection*, uint16_t FLAG);*/
	virtual std::string to_string() const;

protected:
	void set_state(TCP::Connection*, State&);

}; // < TCP::Connection::State

class State_V2 {
public:
	inline virtual void receive(TCP::Connection* tcp, Packet& p) {
		tcp.drop(p);
	}
	inline virtual void active_open(TCP::Connection* tcp) {
		tcp.error(TCP::ConnectionException{"Can not open connection."});
	}
	inline virtual void passive_open(TCP::Connection* tcp) {
		tcp.error(TCP::ConnectionException{"Can not open connection."});
	}
	inline void send(TCP::Connection* tcp, uint16_t flags) {
		tcp.send(flags);
	}
};

class Listen : public State_V2 {
public:
	inline virtual void receive(TCP::Connection* tcp, Packet p) override {
		if(RST) {
			return;
		}
		if(ACK) {
			send(RST);
			return;
		}
		if(SYN) {
			if(SEG.PRC > TCB.PRC)
				TCB.PRC = SEG.PRC;
			RCV.NEXT = SEG.SEQ+1;
			send(tcp SYN & ACK);
			set_state(tcp, SynSent::instance());
		}
	}
};

class SynSent : public State_V2 {
public:
	inline virtual void receive(TCP::Connection* tcp, Packet p) override {
		if(ACK) {
			if(SEG.ACK =< ISS || SEG.ACK > SND.NEXT) {
				drop();
				return;
			}
		}
		if(RST) {
			drop();
			close();
			return;
		}
	}	
};


///////////////// CONCRETE STATES /////////////////
/*
	CLOSED
*/
class Closed : public State {
public:
	static State& instance();
	/*
		<- Do nothing (Start listening).

		=> Listen.
	*/
	virtual void passive_open(TCP::Connection*);
	/*
		<- Send SYN.

		=> SynSent.
	*/
	virtual void active_open(TCP::Connection*);
private:
	inline Closed() {};
};

/*
	LISTEN
*/
class Listen : public State {
public:
	static State& instance();
	/*
		-> Receive SYN.

		<- Send SYN+ACK.

		=> SynReceived.
	*/
	virtual void synchronize(TCP::Connection* conn) override;
private:
	inline Listen() {};
};

/*
	SYN-SENT
*/
class SynSent : public State {
public:
	inline static State& instance();
	/*
		-> Receive SYN+ACK

		<- Send ACK.

		=> Established.
	*/
	virtual void synchronize(TCP::Connection* conn) override;
private:
	inline SynSent() {};
};

/*
	SYN-RCV
*/
class SynReceived : public State {
public:
	inline static State& instance();

	/*
		-> Receive ACK.

		<- Do nothing (Connection is Established)

		=> Established.
	*/
	virtual void acknowledge(TCP::Connection* conn, Packet& p) override;

private:
	inline SynReceived() {};
};

/*
	ESTABLISHED
*/
class Established : public State {
public:
	inline static State& instance();
	/*
		Every data come with a ACK, right?
	*/
	virtual void acknowledge(TCP::Connection* conn, Packet& p) override;

	/*
		-> Receive FIN.

		<- Send ACK.

		=> CloseWait
	*/
	// What if we wanna close?? => FinWait1
	virtual void close(TCP::Connection* conn) override;

private:
	inline Established() {};
};

/*
	CLOSE-WAIT
*/
class CloseWait : public State {
public:
	inline static State& instance();
	/*
		-> Nothing I think...

		<- Send FIN.
		
		=> LastAck
	*/
	virtual void close(TCP::Connection* conn) override;	

private:
	inline CloseWait() {};
};

/*
	FIN-WAIT-1
*/
class FinWait1 : public State {
public:
	inline static State& instance();
	/*
		-> Receive ACK.

		=> FinWait2.
	*/
	virtual void acknowledge(TCP::Connection* conn, Packet& p) override;

private:
	inline FinWait1() {};
};

/*
	FIN-WAIT-1
*/
class FinWait2 : public State {
public:
	inline static State& instance();
	/*
		
	*/
	

private:
	inline FinWait2() {};
};

/*
	LAST-ACK
*/
class LastAck : public State {
public:
	inline static State& instance();
	/*
		-> Receive ACK.

		<- conn.onClose();

		=> Closed (Tell TCP to remove this connection)
	*/
	virtual void acknowledge(TCP::Connection* conn, Packet& p) override;	
	

private:
	inline LastAck() {};
};

/*
	CLOSING
*/
class Closing : public State {
public:
	inline static State& instance();
	/*
		-> Receive ACK.

		=> TimeWait (Guess this isnt needed, just start a Close-timer)
	*/
	virtual void acknowledge(TCP::Connection* conn, Packet& p) override;	
	

private:
	inline Closing() {};
};

/*
	TIME-WAIT
*/
class TimeWait : public State {
public:
	inline static State& instance();
	/*
		
	*/
	

private:
	inline TimeWait() {};
};


};// < TCP::State
}; // < TCP

#endif