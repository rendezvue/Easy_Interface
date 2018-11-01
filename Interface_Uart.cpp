#include "Interface_Uart.h"
Interface_Uart::Interface_Uart()
{
	m_serial_port = NULL;
}

Interface_Uart::~Interface_Uart()
{

}


int Interface_Uart::Write(char *Out_Buffer, int Length)
{
	Write_Start(Out_Buffer,Length);
	return 1;
}

int Interface_Uart::ReadLen(char *In_Buffer, int Length)
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

void Interface_Uart::Read_Start()
{
	printf("Read_Start!\n");

	m_serial_port->async_read_some(boost::asio::buffer(m_Buffer_Read, MAX_BUFFER_READ),
								boost::bind(&Interface_Uart::Read_Handler,
								this,
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred));
}

void Interface_Uart::Read_Handler(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		printf("READ HANDLER!!!\n");
		
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
		printf("Uart Read Fail!!!!!!\n");
		// do somethings...
	}
}

void Interface_Uart::Write_Start(char *Out_Buffer, int Length)
{
	async_write(*m_serial_port,
							boost::asio::buffer(Out_Buffer, Length),
							boost::bind(&Interface_Uart::Write_Handler, this,
							boost::asio::placeholders::error));
}

void Interface_Uart::Write_Handler(const boost::system::error_code& error)
{
	if (!error)
	{	
	}
	else
	{
		printf("Uart Write Fail!!!!!!\n");
	}
}

void Interface_Uart::Thread_Func()
{
	printf("[%s:%d]\n",__func__,__LINE__);
//	Read_Start();
	printf("[%s:%d]\n",__func__,__LINE__);	
	while(1)
	{
		//uint8_t          cp;
		// --------------------------------------------------------------------------
		//   READ FROM PORT
		// --------------------------------------------------------------------------
		// this function locks the port during read
		//int result = _read_port(cp);
		//int result = asio::read(*m_serial_port, asio::buffer(&cp, 1));
//		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
		char cp;
		int result = asio::read(*m_serial_port, asio::buffer(&cp, 1));

		if( result > 0 )
		{
			boost::lock_guard<boost::mutex> lock(m_mutex);
			{
				m_RcvBuffer.push_back(cp);
			}
			//printf("cp = 0x%x\n",cp);
		}
		else
		{
			
		}
	}
}

bool Interface_Uart::Start(char* comport, int baudrate)
{
	if (m_serial_port == NULL)
	{
		try {
			m_serial_port = new asio::serial_port(m_io_service);
			m_serial_port->open(comport);
			m_serial_port->set_option(asio::serial_port_base::baud_rate(baudrate));
			m_serial_port->set_option(asio::serial_port_base::character_size(8));
			m_serial_port->set_option(asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
			m_serial_port->set_option(asio::serial_port_base::stop_bits(boost::asio::serial_port::stop_bits::one));
			m_Thread = new boost::thread(boost::bind(&Interface_Uart::Thread_Func, this));
			return true;
		}
		catch (boost::exception &e)
		{
			if (m_serial_port != NULL)
			{
				delete m_serial_port;
				m_serial_port = NULL;
			}
			//std::cerr <<  boost::diagnostic_information(e);
			printf("Can't open the comport(%s)\n",comport);
			return false;
		}		
	}
	printf("Telemetry port is already opened.\n");
	return false;
}

void Interface_Uart::Stop()
{
	if (m_serial_port != NULL)
	{

#if defined(_WIN32) || defined(WIN32)
		TerminateThread(m_Thread->native_handle(), 0);
#else
		pthread_cancel(m_Thread->native_handle());
#endif
		m_serial_port->close();
		delete m_serial_port;
		m_serial_port = NULL;
	}	
}

bool Interface_Uart::isAlive()
{
	if ( m_serial_port == NULL )
	{
		return false;
	}
	return true;
}

