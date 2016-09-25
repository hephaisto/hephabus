#include <iostream>
#include <hephabus.hpp>

const uint16_t ENDPOINT = 3;
const uint64_t VALUE = 42;

int main(int argc, char **argv)
{
	boost::asio::io_service io;
	hephabus::Hephabus bus(io, hephabus::DEFAULT_PORT+1);

	hephabus::Message msg(boost::asio::ip::address_v4::loopback(), hephabus::Command::update, ENDPOINT, VALUE);
	bus.send(msg);
}
