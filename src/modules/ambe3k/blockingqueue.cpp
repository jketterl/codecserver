#include "blockingqueue.hpp"
#include "protocol.hpp"

template <class T>
BlockingQueue<T>::BlockingQueue(int size) {
    maxSize = size;
}

template <class T>
void BlockingQueue<T>::push(T item, bool block) {
    std::unique_lock<std::mutex> wlck(writerMutex);
    if (block) {
        while (run && full()) {
            isFull.wait(wlck);
        }
        if (!run) return;
    } else if (full()) {
        throw QueueFullException();
    }
    std::queue<T>::push(item);
    isEmpty.notify_all();
}

template <class T>
bool BlockingQueue<T>::full(){
    return std::queue<T>::size() >= maxSize;
}

template <class T>
T BlockingQueue<T>::pop() {
    std::unique_lock<std::mutex> lck(readerMutex);
    while (run && std::queue<T>::empty()) {
        isEmpty.wait(lck);
    }
    if (!run) return nullptr;
    T value = std::queue<T>::front();
    std::queue<T>::pop();
    if (!full()) {
        isFull.notify_all();
    }
    return value;
}

template <class T>
BlockingQueue<T>::~BlockingQueue<T>() {
    run = false;
    isEmpty.notify_all();
    isFull.notify_all();
}

template class BlockingQueue<Ambe3K::Protocol::Packet*>;
template class BlockingQueue<Ambe3K::Protocol::SpeechPacket*>;