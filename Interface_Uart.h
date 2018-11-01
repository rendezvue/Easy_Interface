#pragma once

#include "Interface_Base.h"

class Interface_Uart: public Interface_Base{
private:
	asio::serial_port *m_serial_port;
	enum { MAX_BUFFER_READ = 1024 };
	char m_Buffer_Read[MAX_BUFFER_READ];

	virtual void Thread_Func() ;
	virtual void Read_Start() ;
	virtual void Read_Handler(const boost::system::error_code& error, size_t bytes_transferred) ;
	virtual void Write_Start(char *Out_Buffer, int Length) ;
	virtual void Write_Handler(const boost::system::error_code& error) ;

public:
	Interface_Uart();
	~Interface_Uart();

	virtual bool Start(char* comport, int baudrate) ;	
	virtual void Stop() ;

	virtual int Write(char *Out_Buffer, int Length) ;
	virtual int ReadLen(char *In_Buffer, int Length) ;

	virtual bool isAlive(); 

/*	virtual bool Start(char* comport, int baudrate);		
	virtual void Stop();	
	virtual int Write(char *Out_Buffer, int Length);
	virtual int ReadLen(char *In_Buffer, int Length);*/

protected:



};

