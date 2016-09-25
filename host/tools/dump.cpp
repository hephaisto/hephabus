#include <iostream>
#include <hephabus.hpp>

void handleMessage(std::shared_ptr<const hephabus::Message> msg)
{
	using hephabus::Command;
	std::cout << "[" << msg->device << "] ";
	switch(msg->command)
	{
		case Command::update:
			std::cout << "UPDATE ";
			break;
		case Command::set:
			std::cout << "SET    ";
			break;
		case Command::modify:
			std::cout << "MODIFY ";
			break;
	}
	std::cout << msg->endpoint << " -> " << msg->data << "\n";
}

int main(int argc, char **argv)
{
	boost::asio::io_service io;
	hephabus::Hephabus bus(io);
	bus.onReceive().connect(handleMessage);
	bus.threadWorker(io);
}
