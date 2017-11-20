// A custom IPC solution to bypass electron IPC.
// Copyright(C) 2017 Streamlabs (General Workings Inc)
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.

#pragma once
#include "ipc.hpp"
#include "ipc-class.hpp"
#include "ipc-message.hpp"
#include "ipc-server-instance.hpp"
#include "os-namedsocket.hpp"
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace IPC {
	class ServerInstance;

	typedef bool(*ServerConnectHandler_t)(void*, OS::ClientId_t);
	typedef void(*ServerDisconnectHandler_t)(void*, OS::ClientId_t);
	typedef void(*ServerMessageHandler_t)(void*, OS::ClientId_t, const std::vector<char>&);

	class Server {
	public:
		Server();
		~Server();

	public: // Status
		void Initialize(std::string socketPath);
		void Finalize();

	public: // Events
		void SetConnectHandler(ServerConnectHandler_t handler, void* data);
		void SetDisconnectHandler(ServerDisconnectHandler_t handler, void* data);
		void SetMessageHandler(ServerMessageHandler_t handler, void* data);

	public: // Functionality
		bool RegisterClass(IPC::Class cls);
		
	protected: // Client -> Server Callback
		void handle_disconnect(OS::ClientId_t clientId);
		void handle_message(OS::ClientId_t clientId, std::vector<char> message);

	private:
		static void WorkerMain(Server* ptr);
		void WorkerLocal();

	private:
		std::unique_ptr<OS::NamedSocket> m_socket;
		bool m_isInitialized = false;
		std::thread m_worker;
		bool m_stopWorker = false;

		// Client management.
		std::mutex m_clientLock;
		std::map<OS::ClientId_t, std::shared_ptr<ServerInstance>> m_clients;
		std::list<std::shared_ptr<ServerInstance>> m_clientsDisconnectDelay;
		std::map<std::string, IPC::Class> m_classes;

		// Event Handlers
		std::pair<ServerConnectHandler_t, void*> m_handlerConnect;
		std::pair<ServerDisconnectHandler_t, void*> m_handlerDisconnect;
		std::pair<ServerMessageHandler_t, void*> m_handlerMessage;
		
		friend class ServerInstance;
	};
}
