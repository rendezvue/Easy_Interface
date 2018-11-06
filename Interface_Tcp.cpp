#include "stdafx.h"
#include "Interface_Tcp.h"
Interface_Tcp::Interface_Tcp()
{
//	m_serial_port = NULL;
	m_Socket = NULL;
}

Interface_Tcp::~Interface_Tcp()
{

}


int Interface_Tcp::Write(char *Out_Buffer, int Length)
{
	Write_Start(Out_Buffer,Length);
	return 1;
}

int Interface_Tcp::ReadLen(char *In_Buffer, int Length)
{
	int ReadNums = 0;
	boost::lock_guard<boost::mutex> lock(m_mutex);
	{
		for( int i = 0 ; i < Length ; i++ )
		{
			if (!m_RcvBuffer.empty())
			{
				ReadNums++;
				char read_data = m_RcvBuffer.front();
				m_RcvBuffer.pop_front();
				In_Buffer[i] = read_data;
			}
			else
			{
				break;
			}
		}
	}
	return ReadNums;
}

void Interface_Tcp::Read_Start()
{
	//printf("Read_Start!\n");
	m_Socket->async_read_some(boost::asio::buffer(m_Buffer_Read, MAX_BUFFER_READ),
		boost::bind(&Interface_Tcp::Read_Handler,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Interface_Tcp::Read_Handler(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		//printf("READ HANDLER!!!\n");		
		boost::lock_guard<boost::mutex> lock(m_mutex);
		{
			for (int i = 0; i < bytes_transferred; i++)
			{
				m_RcvBuffer.push_back(m_Buffer_Read[i]);
			}
		}
		memset(m_Buffer_Read,0,MAX_BUFFER_READ);
		Read_Start();
	}
	else
	{
		printf("TCP Read Fail!!!!!!\n");
		// do somethings...
	}
}

void Interface_Tcp::Write_Start(char *Out_Buffer, int Length)
{
	boost::asio::async_write(*m_Socket,
		boost::asio::buffer(Out_Buffer, Length),
		boost::bind(&Interface_Tcp::Write_Handler,
			this,
			boost::asio::placeholders::error));
}

void Interface_Tcp::Write_Handler(const boost::system::error_code& error)
{
	if (!error)
	{	
		//printf("TCP Write Success!!!!!!!\n");
	}
	else
	{
		printf("TCP Write Fail!!!!!!\n");
	}
}

void Interface_Tcp::Connect_Start(tcp::resolver::iterator endpoint_iterator)
{
	tcp::endpoint endpoint = *endpoint_iterator;
	m_Socket->async_connect(endpoint,
		boost::bind(&Interface_Tcp::Connect_Handler,
			this,
			boost::asio::placeholders::error,
			++endpoint_iterator));
}
void Interface_Tcp::Connect_Handler(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
{
	if (!error) // success, so start waiting for read data 
	{
		Read_Start();
	}
	else if (endpoint_iterator != tcp::resolver::iterator())
	{ // failed, so wait for another connection event 
		m_Socket->close();
		Connect_Start(endpoint_iterator);
	}
}

void Interface_Tcp::Thread_Func()
{
	printf("[%s:%d]\n",__func__,__LINE__);
	tcp::resolver resolver(m_io_service);
	//		tcp::resolver::query query(ipaddr, portnum);
	tcp::resolver::query query(m_Ipaddr.c_str(), m_Portnum.c_str());
	tcp::resolver::iterator iterator = resolver.resolve(query);

	//m_Socket = tcp::socket socket();
	tcp::socket mysocket(m_io_service);
	m_Socket = &mysocket;
	Connect_Start(iterator);
	boost::thread t(boost::bind(&boost::asio::io_service::run, &m_io_service));
	while (1)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		const char* text = "Client Heart beat( Test Code )\n";
		int len = strlen(text);
		//m_Telnet->write_buf((char*)text, len);
	}
}

bool Interface_Tcp::Start(char* ipaddr, char* portnum)
{
	// on Unix POXIS based systems, turn off line buffering of input, so cin.get() returns after every keypress 
// On other systems, you'll need to look for an equivalent 
	try
	{
		m_Ipaddr = std::string(ipaddr);
		m_Portnum = std::string(portnum);
		m_Thread = new boost::thread(boost::bind(&Interface_Tcp::Thread_Func, this));
		// run the IO service as a separate thread, so the main thread can block on standard input 
	}
	catch (boost::exception& e)
	{
		cerr << "Exception: " << "\n";
		//cerr << "Exception: " << e.what() << "\n";
		return false;
	}
	return true;
}

void Interface_Tcp::Stop()
{
	m_Socket->close();
}

bool Interface_Tcp::isAlive()
{
	if (m_Socket != NULL )
	{			
		int received = 0;
		try {
			char buf[100];
			int received = m_Socket->receive(boost::asio::buffer(buf), tcp::socket::message_peek);
		}
		catch(boost::exception &e)
		{
			printf("Check Message Peek ERRER!!, Connection Failed\n");
			return false;
		}				
		return true;
	}
	else
	{
		return false;
	}	
}

