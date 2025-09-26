#include "framebuffer.h"

FrameBuffer::FrameBuffer(int maxSize) : m_maxSize(maxSize) {}

bool FrameBuffer::enqueueFrame(const QImage& frame)
{
    QMutexLocker locker(&m_mutex);
    
    while (m_frameQueue.size() >= m_maxSize) {
        m_bufferNotFull.wait(&m_mutex);
    }
    
    m_frameQueue.enqueue(frame);
    m_bufferNotEmpty.wakeAll();
    return true;
}

QImage FrameBuffer::dequeueFrame()
{
    QMutexLocker locker(&m_mutex);
    
    while (m_frameQueue.isEmpty()) {
        m_bufferNotEmpty.wait(&m_mutex);
    }
    
    QImage frame = m_frameQueue.dequeue();
    m_bufferNotFull.wakeAll();
    return frame;
}

int FrameBuffer::queueSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_frameQueue.size();
}

void FrameBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_frameQueue.clear();
}
