#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define CIRCULAR_BUFFER_MINIMUM_SIZE        0x80

template<class T>
class CircularBuffer
{
public:
    CircularBuffer() = delete;
    CircularBuffer(size_t capacity) noexcept;
    CircularBuffer(const CircularBuffer& source) noexcept;
    CircularBuffer(CircularBuffer&& source) noexcept;
    ~CircularBuffer() noexcept;
    CircularBuffer& operator=(const CircularBuffer& source) noexcept;
    const T& operator[] (size_t index) const;
    size_t getCapacity() const { return mSize; };
    size_t getAvailable() const { return mHead <= mTail ? mTail - mHead : mSize - mHead + mTail; };
    size_t push(const T& item);
    size_t push(const T* items, size_t size);
    size_t pop(T* items, size_t size);
    void clear();
    size_t drop(size_t size);
    bool empty() const { return mHead == mTail; };
private:
    size_t          mSize;
    T*              mBuffer;
    size_t          mHead;
    size_t          mTail;
};

template<class T>
CircularBuffer<T>::CircularBuffer(size_t capacity) noexcept :
    mSize(capacity < CIRCULAR_BUFFER_MINIMUM_SIZE ? CIRCULAR_BUFFER_MINIMUM_SIZE : capacity),
    mBuffer(new T[mSize]),
    mHead(0),
    mTail(0)
{}

template<class T>
CircularBuffer<T>::CircularBuffer(const CircularBuffer& source) noexcept :
    mSize(source.mSize),
    mBuffer(new T[mSize]),
    mHead(source.mHead),
    mTail(source.mTail)
{
    for (size_t i = mHead; i < mTail; i++)
        mBuffer[i] = source.mBuffer[i];
}

template<class T>
CircularBuffer<T>::CircularBuffer(CircularBuffer&& source) noexcept :
    mSize(source.mSize),
    mBuffer(source.mBuffer),
    mHead(source.mHead),
    mTail(source.mTail)
{
    source.mBuffer = nullptr;
}


template<class T>
CircularBuffer<T>::~CircularBuffer() noexcept
{
    if (mBuffer != nullptr)
        delete[] mBuffer;
}

template<class T>
CircularBuffer<T>& CircularBuffer<T>::operator=(const CircularBuffer& source) noexcept
{
    if (this != &source)
    {
        if (this->mBuffer != nullptr)
            delete[] this->mBuffer;
        this->mSize = source.mSize;
        this->mBuffer = new T[this->mSize];
        this->mHead = source.mHead;
        this->mTail = source.mTail;
        for (size_t i = mHead; i < mTail; i++)
            mBuffer[i] = source.mBuffer[i];
    }
    return *this;
}

template<class T>
const T& CircularBuffer<T>::operator[] (size_t index) const
{
    return mBuffer[(mHead + index) % mSize];
}

template<class T>
size_t CircularBuffer<T>::push(const T& item)
{
    if ((mTail + 1) % mSize != mHead)
    {
        mBuffer[mTail] = item;
        mTail = (mTail + 1) % mSize;
        return 1;
    }
    return 0;
}

template<class T>
size_t CircularBuffer<T>::push(const T* items, size_t size)
{
    int sizeToPush = (int)(mSize - getAvailable());
    if (sizeToPush > size)
        sizeToPush = (int)size;
    for (size_t i = 0; i < sizeToPush; i++)
    {
        mBuffer[mTail] = items[i];
        mTail = (mTail + 1) % mSize;
    }
    return sizeToPush;
}

template<class T>
size_t CircularBuffer<T>::pop(T* items, size_t size)
{
    size_t popSize = getAvailable();
    if (size < popSize)
        popSize = size;
    for (size_t i = 0; i < popSize; i++)
    {
        items[i] = mBuffer[mHead];
        mHead = (mHead + 1) % mSize;
    }
    return popSize;
}

template<class T>
void CircularBuffer<T>::clear()
{
    mHead = 0;
    mTail = 0;
}

template<class T>
size_t CircularBuffer<T>::drop(size_t size)
{
    size_t dropSize = getAvailable();
    if (size < dropSize)
        dropSize = size;
    mHead = (mHead + dropSize) % mSize;
    return dropSize;
}

#endif // CIRCULAR_BUFFER_H