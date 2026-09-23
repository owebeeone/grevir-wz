#include <GrevirPacket.h>
#include <string.h>

extern "C" {
volatile std::uint8_t packet_result = 0;
volatile std::uint8_t packet_frames = 0;
volatile std::uint8_t packet_deliveries = 0;
volatile std::uint8_t packet_seed = 1;
}

int main() {
  ardo::FragmentSender<2, 8> sender;
  ardo::FragmentReceiver<1, 2, 8> receiver;
  const std::uint8_t payload[9]{packet_seed, 2, 3, 4, 5, 6, 7, 8, 9};
  bool matched = false;
  const bool sent = sender.send(payload, sizeof(payload),
    [&](const std::uint8_t* frame, std::uint32_t frame_length) {
      packet_frames = static_cast<std::uint8_t>(packet_frames + 1);
      receiver.receive(std::uint32_t{1}, 1234, frame, frame_length,
        [&](const std::uint8_t* data, std::uint32_t length) {
          packet_deliveries = static_cast<std::uint8_t>(packet_deliveries + 1);
          matched = length == sizeof(payload) && ::memcmp(data, payload, sizeof(payload)) == 0;
        });
    });
  packet_result = sent && matched && packet_frames == 2 && packet_deliveries == 1
    ? 0xa5 : 0x5a;
  while (true) {
  }
}
