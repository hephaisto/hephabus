#include <memory>
#include <string>

#include <boost/signals2.hpp>
#include <boost/asio.hpp>

namespace hephabus
{

namespace bs2 = boost::signals2;
namespace asio = boost::asio;
using asio::ip::udp;


typedef asio::ip::address DeviceAddress;

enum class Command : uint8_t
{
	update,
	set,
	modify
};

extern const uint16_t DEFAULT_PORT;

class Message
{
public:
	Message(DeviceAddress device, uint8_t *buf);
	Message(DeviceAddress target, Command command, uint16_t endpoint, int64_t data);
	void fillInBuffer(uint8_t *buf) const;

	static const size_t COMMAND_POS = 0;
	static const size_t ENDPOINT_POS = COMMAND_POS + 1;
	static const size_t DATA_POS = ENDPOINT_POS + 2;
	static const size_t MESSAGE_LEN = DATA_POS + 8;

	const Command command;
	const uint16_t endpoint;
	const DeviceAddress device;
	const int64_t data;
};

typedef bs2::signal<void (std::shared_ptr<const Message>)> ReceiveSignal;

class Hephabus
{
public:
	Hephabus(boost::asio::io_service &io, uint16_t port = DEFAULT_PORT);
	void threadWorker(boost::asio::io_service &io);

	ReceiveSignal& onReceive();

	void send(const Message &msg);

private:
	asio::ip::udp::socket socket;
	void startWaitingForPacket();
	void handleReceive(const boost::system::error_code& error, size_t len);

	ReceiveSignal _onReceive;

	// receive buffers
	uint8_t buffer[Message::MESSAGE_LEN];
	udp::endpoint remote_udp_endpoint;
};

typedef std::function<void (int64_t)> UpdateHandler;

class HephabusUpdateReceiver
{
public:
	HephabusUpdateReceiver(boost::asio::io_service &io, uint16_t port=DEFAULT_PORT);
	void registerHandler(DeviceAddress address, uint16_t endpoint, UpdateHandler handler);
private:
	Hephabus bus;
	void handleReceive(std::shared_ptr<const Message> msg);
	std::map<std::tuple<DeviceAddress, uint16_t>, UpdateHandler> handlers;
};

} // namespace hephabus
