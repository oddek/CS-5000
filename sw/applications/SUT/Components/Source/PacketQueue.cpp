#include "PacketQueue.h"

template <QueueType Type>
void PacketQueue<Type>::put(const Packet<>& packet)
{
    if constexpr (Type == QueueType::Blocking)
    {
        pthread_mutex_lock(&mutex);
    }

    queue.push(packet);
    updateMaxSize();

    if constexpr (Type == QueueType::Blocking)
    {
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&mutex);
    }
}

template <QueueType Type>
size_t PacketQueue<Type>::getMaxSize() const
{
    return maxSize;
}

template <QueueType Type>
void PacketQueue<Type>::updateMaxSize()
{
    if( queue.size() > maxSize )
    {
        maxSize = queue.size();
    }
}

template <QueueType Type>
Packet<> PacketQueue<Type>::get()
{
    if( isEmpty())
    {
        throw std::runtime_error("Get called on empty queue");
    }

    if constexpr (Type == QueueType::Blocking)
    {
        pthread_mutex_lock(&mutex);
        while(isEmpty())
        {
            pthread_cond_wait(&condition, &mutex);
        }
    }

    auto packet = queue.front();
    queue.pop();
    if constexpr (Type == QueueType::Blocking)
    {
        pthread_mutex_unlock(&mutex);
    }

    return packet;
}

template <QueueType Type>
bool PacketQueue<Type>::isEmpty() const
{
    return queue.empty();
}

template <QueueType Type>
void PacketQueue<Type>::clear()
{
    while(!queue.empty())
    {
        queue.pop();
    }
}

template <QueueType Type>
size_t PacketQueue<Type>::getSize() const
{
    return queue.size();
}

template class PacketQueue<QueueType::Blocking>;
template class PacketQueue<QueueType::NonBlocking>;
