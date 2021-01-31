// Core/System/Socket.cpp

#include <Core/System/Socket.h>

#include <Core/Platform.h>

#include <asio.hpp>

#include <cassert>

#ifdef IS_WINDOWS
#include <Winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

namespace Core
{
	namespace detail
	{
		class SocketTimeoutSettingError : public asio::error_category
		{
		public:
			~SocketTimeoutSettingError() override {}
			const char* name() const noexcept override { return "SocketTimeoutSettingError"; }
			std::string message(int ev) const override { return "Failed to set the socket timeout with a platofrm-dependent system API call."; }
		};

		class ClientConnectingError : public asio::error_category
		{
		public:
			~ClientConnectingError() override {}
			const char* name() const noexcept override { return "ClientConnectingError"; }
			std::string message(int ev) const override  { return "The socket client has failed to connect to the server."; }
		};

		class SocketImplementor
		{
		protected:

			asio::io_service m_ASIOService;
			asio::ip::tcp::socket m_Socket;
			asio::error_code m_LastError;

		public:

			SocketImplementor()
				: m_Socket(m_ASIOService)
			{
			}

			void Close()
			{
				m_Socket.close();
			}

			bool HasError() const
			{
				return static_cast<bool>(m_LastError.value());
			}

			void ClearError()
			{
				m_LastError.clear();
			}

			void Send(const void* buffer, size_t size)
			{
				asio::write(m_Socket, asio::buffer(buffer, size), m_LastError);
			}

			size_t Receive(void* buffer, unsigned bufferSize)
			{
				return m_Socket.read_some(asio::buffer(buffer, bufferSize), m_LastError);
			}

			void ReceiveWithRequestedSize(void* buffer, unsigned requestedSize)
			{
				size_t readByteCount = 0;
				auto byteBuffer = reinterpret_cast<unsigned char*>(buffer);
				while (readByteCount < requestedSize)
				{
					size_t currentlyReadBytes = m_Socket.read_some(
						asio::buffer(byteBuffer + readByteCount, requestedSize - readByteCount),
						m_LastError);
					readByteCount += currentlyReadBytes;
					if (HasError()) break;
					assert(HasError() || currentlyReadBytes != 0);
				}
				assert(HasError() || readByteCount == requestedSize);
			}

			void SetTimeout(unsigned milliseconds)
			{
				if (milliseconds == 0) return;
				int ret = 0;
#ifdef IS_WINDOWS
				ret |= setsockopt(m_Socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&milliseconds,
					sizeof(milliseconds));
				ret |= setsockopt(m_Socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&milliseconds,
					sizeof(milliseconds));
#else
				struct timeval tv;
				tv.tv_sec = milliseconds / 1000;
				tv.tv_usec = milliseconds % 1000;
				ret |= setsockopt(m_Socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				ret |= setsockopt(m_Socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
				if (ret != 0) m_LastError = asio::error_code(1, SocketTimeoutSettingError());
			}
		};

		class ServerSocketImplementor : public SocketImplementor
		{
		public:

			void Start(unsigned short portNumber)
			{
				asio::ip::tcp::acceptor acceptor(m_ASIOService,
					asio::ip::tcp::endpoint(asio::ip::tcp::v4(), portNumber));
				acceptor.accept(m_Socket, m_LastError);
			}
		};

		class ClientSocketImplementor : public SocketImplementor
		{
		public:

			void Start(const std::string& hostName, unsigned short portNumber)
			{
				std::stringstream ssPortNumber;
				ssPortNumber << portNumber;

				asio::ip::tcp::resolver resolver(m_ASIOService);
				asio::ip::tcp::resolver::query query(
					asio::ip::tcp::v4(), hostName, ssPortNumber.str());
				asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query, m_LastError);
				asio::connect(m_Socket, iterator, m_LastError);
			}

			void SetConnectingError()
			{
				m_LastError = asio::error_code(1, ClientConnectingError());
			}
		};
	}
}

using namespace Core;

ServerSocket::ServerSocket()
	: m_Implementor(std::make_unique<detail::ServerSocketImplementor>())
{
}

ServerSocket::~ServerSocket()
{
}

bool ServerSocket::HasError() const
{
	return m_Implementor->HasError();
}

void ServerSocket::ClearError()
{
	m_Implementor->ClearError();
}

void ServerSocket::Close()
{
	m_Implementor->Close();
}

void ServerSocket::Start(unsigned short portNumber)
{
	m_Implementor->Start(portNumber);
}

void ServerSocket::Send(const void* buffer, size_t size)
{
	m_Implementor->Send(buffer, size);
}

size_t ServerSocket::Receive(void* buffer, unsigned bufferSize)
{
	return m_Implementor->Receive(buffer, bufferSize);
}

void ServerSocket::ReceiveWithRequestedSize(void* buffer, unsigned requestedSize)
{
	m_Implementor->ReceiveWithRequestedSize(buffer, requestedSize);
}

void ServerSocket::SetTimeout(unsigned milliseconds)
{
	m_Implementor->SetTimeout(milliseconds);
}

ClientSocket::ClientSocket()
	: m_Implementor(std::make_unique<detail::ClientSocketImplementor>())
{
}

ClientSocket::~ClientSocket()
{
}

bool ClientSocket::HasError() const
{
	return m_Implementor->HasError();
}

void ClientSocket::ClearError()
{
	m_Implementor->ClearError();
}

void ClientSocket::Start(const std::string& hostName, unsigned short portNumber)
{
	m_Implementor->Start(hostName, portNumber);
}

bool ClientSocket::StartWithMultipleConnectAttempts(const std::string& hostName,
	unsigned short portNumber, std::atomic<bool>* pStop)
{
	while (true)
	{
		m_Implementor->ClearError();
		if (pStop != nullptr && pStop->load()) return false;
		try
		{
			m_Implementor->Start(hostName, portNumber);
			if (!m_Implementor->HasError()) return true;
		}
		catch (...) {}
	}
}

void ClientSocket::Close()
{
	m_Implementor->Close();
}

void ClientSocket::Send(const void* buffer, size_t size)
{
	m_Implementor->Send(buffer, size);
}

size_t ClientSocket::Receive(void* buffer, unsigned bufferSize)
{
	return m_Implementor->Receive(buffer, bufferSize);
}

void ClientSocket::ReceiveWithRequestedSize(void* buffer, unsigned requestedSize)
{
	m_Implementor->ReceiveWithRequestedSize(buffer, requestedSize);
}

void ClientSocket::SetTimeout(unsigned milliseconds)
{
	m_Implementor->SetTimeout(milliseconds);
}
