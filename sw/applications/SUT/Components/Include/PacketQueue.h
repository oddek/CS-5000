
#ifndef SUT_PACKETQUEUE_H
#define SUT_PACKETQUEUE_H

#include "Packet.h"
#include <queue>

enum class QueueType
{
    NonBlocking,
    Blocking
};

template<QueueType Type = QueueType::NonBlocking>
class PacketQueue
{
  public:
    void put(const Packet<>& packet);
    Packet<> get();
    void clear();
    void updateMaxSize();

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] size_t getSize() const;
    [[nodiscard]] size_t getMaxSize() const;

  private:
    pthread_mutex_t mutex{};
    pthread_cond_t condition{};
    std::queue<Packet<>> queue{};
    size_t maxSize{0};
};

#endif // SUT_PACKETQUEUE_H
