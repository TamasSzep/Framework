// Core/System/Socket.h

#pragma once

#include <memory>
#include <atomic>
#include <string>

namespace Core
{
	namespace detail
	{
		class ServerSocketImplementor;
		class ClientSocketImplementor;
	}

	class ServerSocket
	{
		std::unique_ptr<detail::ServerSocketImplementor> m_Implementor;

	public:

		ServerSocket();
		~ServerSocket();

		bool HasError() const;
		void ClearError();

		void Start(unsigned short portNumber);
		void Close();

		void Send(const void* buffer, size_t size);
		size_t Receive(void* buffer, unsigned bufferSize);
		void ReceiveWithRequestedSize(void* buffer, unsigned requestedSize);
		
		void SetTimeout(unsigned milliseconds);
	};

	class ClientSocket
	{
		std::unique_ptr<detail::ClientSocketImplementor> m_Implementor;

	public:

		ClientSocket();
		~ClientSocket();

		bool HasError() const;
		void ClearError();

		void Start(const std::string& hostName, unsigned short portNumber);
		bool StartWithMultipleConnectAttempts(const std::string& hostName,
			unsigned short portNumber, std::atomic<bool>* pStop = nullptr);
		void Close();
		
		void Send(const void* buffer, size_t size);
		size_t Receive(void* buffer, unsigned bufferSize);
		void ReceiveWithRequestedSize(void* buffer, unsigned requestedSize);
	
		void SetTimeout(unsigned milliseconds);
	};
}
