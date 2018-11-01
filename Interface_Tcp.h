#pragma once

#include "Interface_Base.h"

class Interface_Tcp: public Interface_Base {
private:	
	tcp::socket *m_Socket; // the socket this instance is connected to 
	std::string m_Ipaddr;
	std::string m_Portnum;
	enum { MAX_BUFFER_READ = 1024 };
	char m_Buffer_Read[MAX_BUFFER_READ];	
private:
	void Connect_Start(tcp::resolver::iterator endpoint_iterator);
	void Connect_Handler(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
private:

	virtual void Thread_Func() ;
	virtual void Read_Start() ;
	virtual void Read_Handler(const boost::system::error_code& error, size_t bytes_transferred) ;
	virtual void Write_Start(char *Out_Buffer, int Length) ;
	virtual void Write_Handler(const boost::system::error_code& error) ;

public:
	Interface_Tcp();
	~Interface_Tcp();

	virtual bool Start(char* comport, int baudrate) { return false; }
	virtual bool Start(char* ipaddr, char* portnum);

	virtual void Stop() ;

	virtual int Write(char *Out_Buffer, int Length) ;
	virtual int ReadLen(char *In_Buffer, int Length) ;

	virtual bool isAlive(); 

protected:

};

