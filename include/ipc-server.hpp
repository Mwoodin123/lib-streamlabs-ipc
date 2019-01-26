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
#include "ipc-server-instance.hpp"
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#ifdef WIN32
#include "../source/windows/named-pipe.hpp"
#endif

namespace ipc {
	class server_instance;

	typedef bool(*server_connect_handler_t)(void*, int64_t);
	typedef void(*server_disconnect_handler_t)(void*, int64_t);
	typedef void(*server_message_handler_t)(void*, int64_t, const std::vector<char>&);

	class server {
		bool m_isInitialized = false;

		// Functions		
		std::map<std::string, std::shared_ptr<ipc::collection>> m_classes;

		// Socket
		size_t backlog = 40;
		std::mutex m_sockets_mtx;
#ifdef WIN32
		std::list<std::shared_ptr<os::windows::named_pipe>> m_sockets;
#endif
		std::string m_socketPath = "";

		// Client management.
		std::mutex m_clients_mtx;
#ifdef WIN32
		std::map<std::shared_ptr<os::windows::named_pipe>, std::shared_ptr<server_instance>> m_clients;
#endif

		// Event Handlers
		std::pair<server_connect_handler_t, void*> m_handlerConnect;
		std::pair<server_disconnect_handler_t, void*> m_handlerDisconnect;
		std::pair<server_message_handler_t, void*> m_handlerMessage;

		// Worker
		struct {
			std::thread worker;
			bool stop = false;
		} m_watcher;

		void watcher();

#ifdef WIN32
		void spawn_client(std::shared_ptr<os::windows::named_pipe> socket);
		void kill_client(std::shared_ptr<os::windows::named_pipe> socket);
#endif

		public:
		server();
		~server();

		public: // Status
		void initialize(std::string socketPath);
		void finalize();

		public: // Events
		void set_connect_handler(server_connect_handler_t handler, void* data);
		void set_disconnect_handler(server_disconnect_handler_t handler, void* data);
		void set_message_handler(server_message_handler_t handler, void* data);

		public: // Functionality
		bool register_collection(std::shared_ptr<ipc::collection> cls);

		protected: // Client -> Server
		bool client_call_function(int64_t cid, std::string cname, std::string fname,
			std::vector<ipc::value>& args, std::vector<ipc::value>& rval, std::string& errormsg);

		friend class server_instance;
	};
}
