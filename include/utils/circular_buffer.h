#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <cstddef>

template<class T>
class CircularBuffer
{
public:
  constexpr static size_t CIRCULAR_BUFFER_MINIMUM_SIZE = 0x80;
  CircularBuffer() = delete;
  explicit CircularBuffer(size_t capacity);
  CircularBuffer(const CircularBuffer& source);
  ~CircularBuffer() noexcept;
  CircularBuffer& operator=(const CircularBuffer& source);
  const T& operator[] (size_t index) const;
  size_t get_capacity() const { return size_; };
  size_t get_available() const { return head_ <= tail_ ? tail_ - head_ : size_ - head_ + tail_; };
  size_t push(const T& item);
  size_t push(const T* items, size_t size);
  size_t pop(T* items, size_t size);
  void clear();
  size_t drop(size_t size);
  bool empty() const { return head_ == tail_; };
private:
  size_t          size_;
  T*              buffer_;
  size_t          head_;
  size_t          tail_;
};

template<class T>
CircularBuffer<T>::CircularBuffer(size_t capacity) :
  size_(capacity < CIRCULAR_BUFFER_MINIMUM_SIZE ? CIRCULAR_BUFFER_MINIMUM_SIZE : capacity),
  buffer_(new T[size_]),
  head_(0),
  tail_(0)
{}

template<class T>
CircularBuffer<T>::CircularBuffer(const CircularBuffer& source) :
  size_(source.size_),
  buffer_(new T[size_]),
  head_(source.head_),
  tail_(source.tail_)
{
  for (size_t i = this->head_; i < this->tail_; i++)
    this->buffer_[i] = source.buffer_[i];
}

template<class T>
CircularBuffer<T>::~CircularBuffer() noexcept
{
  delete[] this->buffer_;
}

template<class T>
CircularBuffer<T>& CircularBuffer<T>::operator=(const CircularBuffer& source)
{
  if (this != &source)
  {
    delete[] this->buffer_;
    this->size_ = source.size_;
    this->buffer_ = new T[this->size_];
    this->head_ = source.head_;
    this->tail_ = source.tail_;
    for (size_t i = this->head_; i < this->tail_; i++)
      this->buffer_[i] = source.buffer_[i];
  }
  return *this;
}

template<class T>
const T& CircularBuffer<T>::operator[] (size_t index) const
{
    return this->buffer_[(this->head_ + index) % this->size_];
}

template<class T>
size_t CircularBuffer<T>::push(const T& item)
{
    if ((this->tail_ + 1) % this->size_ != this->head_)
    {
        this->buffer_[this->tail_] = item;
        this->tail_ = (this->tail_ + 1) % this->size_;
        return 1;
    }
    return 0;
}

template<class T>
size_t CircularBuffer<T>::push(const T* items, size_t size)
{
    size_t size_to_push = (int)(this->size_ - this->get_available());
    if (size_to_push > size)
        size_to_push = size;
    for (size_t i = 0; i < size_to_push; i++)
    {
        this->buffer_[tail_] = items[i];
        this->tail_ = (this->tail_ + 1) % this->size_;
    }
    return size_to_push;
}

template<class T>
size_t CircularBuffer<T>::pop(T* items, size_t size)
{
    size_t pop_size = this->get_available();
    if (size < pop_size)
        pop_size = size;
    for (size_t i = 0; i < pop_size; i++)
    {
        items[i] = this->buffer_[this->head_];
        this->head_ = (this->head_ + 1) % this->size_;
    }
    return pop_size;
}

template<class T>
void CircularBuffer<T>::clear()
{
    this->head_ = 0;
    this->tail_ = 0;
}

template<class T>
size_t CircularBuffer<T>::drop(size_t size)
{
    size_t drop_size = this->get_available();
    if (size < drop_size)
        drop_size = size;
    this->head_ = (this->head_ + drop_size) % this->size_;
    return drop_size;
}

#endif // CIRCULAR_BUFFER_H