#include "Server.hh"

#include <algorithm>

Server::Server()
{
	server.set_access_channels(websocketpp::log::alevel::all);
	server.clear_access_channels(websocketpp::log::alevel::frame_payload);
	server.init_asio();

	server.set_message_handler([this](Connection conn, Message msg)
	{
		//	TODO add a rate limiter

		try
		{
			std::stringstream received(msg->get_payload());

			std::string cmd;
			received >> cmd;

			//	If the connection is in a room, forward the message to that room
			Room* room = findRoom(conn);
			if(room) room->handleMessage(conn, cmd, received);

			else if(cmd == "list")
			{
				std::ostringstream roomData;
				roomData << "list";

				for(auto& room : rooms)
					roomData << ' ' << room.first;

				server.send(conn, roomData.str(), websocketpp::frame::opcode::text);
			}

			else if(cmd == "create")
			{
				std::string roomName;
				received >> roomName;

				//	If the name is empty, ignore the message
				if(roomName.empty())
					return;

				//	If the room already exists, inform the user
				if(rooms.find(roomName) != rooms.end())
				{
					server.send(conn, "room-exists", websocketpp::frame::opcode::text);
					return;
				}

				server.send(conn, "create", websocketpp::frame::opcode::text);

				//	Add a new room and give this connection to it
				auto room = rooms.emplace(roomName, Room(server));
				room.first->second.addConnection(conn);
			}

			else if(cmd == "join")
			{
				std::string roomName;
				received >> roomName;

				auto it = rooms.find(roomName);

				//	Does the room exist?
				if(it == rooms.end())
				{
					server.send(conn, "invalid-room", websocketpp::frame::opcode::text);
					return;
				}

				server.send(conn, "join", websocketpp::frame::opcode::text);

				//	Add the connection to the given room
				it->second.addConnection(conn);
			}

			else server.send(conn, "invalid", websocketpp::frame::opcode::text);
		}

		catch (websocketpp::exception const & e)
		{
			std::cout << "Echo failed because: "
					  << "(" << e.what() << ")" << std::endl;
		}
	});

	server.set_reuse_addr(true);
	server.listen(9002);
	server.start_accept();
	server.run();
}

Room* Server::findRoom(Connection& conn)
{
	for(auto& room : rooms)
	{
		if(room.second.connectionHere(conn))
			return &room.second;
	}

	return nullptr;
}
