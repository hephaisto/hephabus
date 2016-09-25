#include "../include/hephabus.hpp"

#include <boost/endian/conversion.hpp>

#include "../include/constants.h"

namespace hephabus
{

Command int_to_command(uint8_t value)
{
	switch(value)
	{
		case HEPHABUS_CMD_UPDATE:
			return Command::update;
		case HEPHABUS_CMD_SET:
			return Command::set;
		case HEPHABUS_CMD_MODIFY:
			return Command::modify;
		default:
			throw std::runtime_error("invalid command value");
	}
}

Message::Message(DeviceAddress device, uint8_t *buf)
:Message(
	device,
	int_to_command(buf[0]),
	boost::endian::big_to_native( *( (uint16_t*) (buf+ENDPOINT_POS) ) ),
	boost::endian::big_to_native( *( (int64_t*) (buf+DATA_POS) ) )
)
{
}

Message::Message(DeviceAddress device, Command command, uint16_t endpoint, int64_t data)
:device(device),
command(command),
endpoint(endpoint),
data(data)
{
}

void Message::fillInBuffer(uint8_t *buf) const
{
	buf[0] = static_cast<uint8_t>(command);
	*(uint16_t*)(buf+ENDPOINT_POS) = boost::endian::native_to_big(endpoint);
	*(uint64_t*)(buf+DATA_POS) = boost::endian::native_to_big(data);
}

Hephabus::Hephabus(boost::asio::io_service &io, uint16_t port)
:socket(io, udp::endpoint(udp::v4(), port))
{
	startWaitingForPacket();
}

void Hephabus::threadWorker(boost::asio::io_service &io)
{
	io.run();
}

void Hephabus::startWaitingForPacket()
{
	socket.async_receive_from(
		boost::asio::buffer(buffer),
		remote_udp_endpoint,
		boost::bind(&Hephabus::handleReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);
}

void Hephabus::handleReceive(const boost::system::error_code& error, size_t len)
{
	if(!error)
	{
		if(len != Message::MESSAGE_LEN)
			throw std::runtime_error("error while reading udp data: message too short");
		std::shared_ptr<const Message> msg(new Message(remote_udp_endpoint.address(), buffer));
		_onReceive(msg);
	}
	else
		throw std::runtime_error("error while reading udp data: "+error.message());
	startWaitingForPacket();
}

void Hephabus::send(const Message &msg)
{
	uint8_t buf[Message::MESSAGE_LEN];
	msg.fillInBuffer(buf);

	socket.async_send_to(
		asio::buffer(buf),
		udp::endpoint(msg.device, DEFAULT_PORT),
		[](const boost::system::error_code&, const size_t){}
	);
}

ReceiveSignal& Hephabus::onReceive()
{
	return _onReceive;
}

HephabusUpdateReceiver::HephabusUpdateReceiver(asio::io_service &io, uint16_t port)
:bus(io, port)
{
	bus.onReceive().connect(std::bind(&HephabusUpdateReceiver::handleReceive, this, std::placeholders::_1));
}

void HephabusUpdateReceiver::registerHandler(DeviceAddress address, uint16_t endpoint, UpdateHandler handler)
{
	handlers.insert(std::make_pair(std::make_tuple(address, endpoint), handler));
}

void HephabusUpdateReceiver::handleReceive(std::shared_ptr<const Message> msg)
{
	if(msg->command == Command::update)
		for( auto it: handlers)
		{
			auto match_spec = it.first;
			if( (std::get<0>(match_spec) == msg->device) && (std::get<1>(match_spec) == msg->endpoint) )
				it.second(msg->data);
		}
}

const uint16_t DEFAULT_PORT = HEPHABUS_DEFAULT_PORT;

}
