#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <cstddef>
#include <algorithm>

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
  size_t get_capacity() const { return capacity_; };
  size_t get_size() const;
  size_t get_space() const { return this->capacity_ - this->get_size(); };
  size_t push(const T& item);
  size_t push(const T* items, size_t size);
  size_t pop(T* items, size_t size);
  T* reserve(size_t& size);
  void clear();
  size_t drop(size_t size);
  bool empty() const { return is_empty_; };
private:
  bool            is_empty_;
  size_t          capacity_;
  T*              buffer_;
  size_t          head_;
  size_t          tail_;
};

template<class T>
CircularBuffer<T>::CircularBuffer(size_t capacity) :
  is_empty_(true),
  capacity_(capacity < CIRCULAR_BUFFER_MINIMUM_SIZE ? CIRCULAR_BUFFER_MINIMUM_SIZE : capacity),
  buffer_(new T[capacity_]),
  head_(0),
  tail_(0)
{}

template<class T>
CircularBuffer<T>::CircularBuffer(const CircularBuffer& source) :
  is_empty_(source.is_empty_),
  capacity_(source.capacity_),
  buffer_(new T[capacity_]),
  head_(source.head_),
  tail_(source.tail_)
{
  size_t size = source.get_size();
  for (size_t i = 0; i < size; i++)
  {
    size_t ind = (this->head_ + i) % this->capacity_;
    this->buffer_[ind] = source.buffer_[ind];
  }
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
    this->capacity_ = source.capacity_;
    this->buffer_ = new T[this->capacity_];
    this->head_ = source.head_;
    this->tail_ = source.tail_;
    size_t size = source.get_size();
    for (size_t i = 0; i < size; i++)
    {
      size_t ind = (this->head_ + i) % this->capacity_;
      this->buffer_[ind] = source.buffer_[ind];
    }
  }
  return *this;
}

template<class T>
const T& CircularBuffer<T>::operator[] (size_t index) const
{
    return this->buffer_[(this->head_ + index) % this->capacity_];
}

template<class T>
size_t CircularBuffer<T>::get_size() const
{
  if (this->tail_ == this->head_)
    return this->is_empty_ ? 0 : this->capacity_;
  return this->head_ < this->tail_ ? this->tail_ - this->head_ : this->capacity_ - this->head_ + this->tail_;
}

template<class T>
size_t CircularBuffer<T>::push(const T& item)
{
    if (this->is_empty_ || (this->tail_ != this->head_))
    {
        this->buffer_[this->tail_] = item;
        this->tail_ = (this->tail_ + 1) % this->capacity_;
        this->is_empty_ = false;
        return 1;
    }
    return 0;
}

template<class T>
size_t CircularBuffer<T>::push(const T* items, size_t size)
{
    size_t size_to_push = this->get_space();
    if (size_to_push > size)
        size_to_push = size;
    for (size_t i = 0; i < size_to_push; i++)
    {
        this->buffer_[tail_] = items[i];
        this->tail_ = (this->tail_ + 1) % this->capacity_;
    }
    if (size_to_push > 0)
      this->is_empty_ = false;
    return size_to_push;
}

template<class T>
size_t CircularBuffer<T>::pop(T* items, size_t size)
{
    size_t sz = this->get_size();
    size_t pop_size = sz;
    if (size < pop_size)
        pop_size = size;
    for (size_t i = 0; i < pop_size; i++)
    {
        items[i] = this->buffer_[this->head_];
        this->head_ = (this->head_ + 1) % this->capacity_;
    }
    if (pop_size == sz)
      this->is_empty_ = true;
    return pop_size;
}

template<class T>
T* CircularBuffer<T>::reserve(size_t& size)
{
    size_t new_size;
    size_t head = this->head_ == 0 ? this->capacity_ : 0;
    if (head > this->tail_)
        new_size = std::min(size, head - this->tail_);
    else
        new_size = std::min(size, this->capacity_ - this->tail_);
    T* result = this->buffer_ + this->tail_;
    this->tail_ = (this->tail_ + new_size) %  this->capacity_;
    size = new_size;
    if (new_size > 0)
      this->is_empty_ = false;
    return result;
}

template<class T>
void CircularBuffer<T>::clear()
{
    this->is_empty_ = true;
    this->head_ = 0;
    this->tail_ = 0;
}

template<class T>
size_t CircularBuffer<T>::drop(size_t size)
{
    size_t sz = this->get_size();
    size_t drop_size = sz;
    if (size < drop_size)
        drop_size = size;
    this->head_ = (this->head_ + drop_size) % this->capacity_;
    if (drop_size == sz)
      this->is_empty_ = true;;
    return drop_size;
}

#endif // CIRCULAR_BUFFER_H