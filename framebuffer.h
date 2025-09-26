#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>

class FrameBuffer
{
public:
    explicit FrameBuffer(int maxSize = 30);
    
    bool enqueueFrame(const QImage& frame);
    QImage dequeueFrame();
    int queueSize() const;
    void clear();

private:
    QQueue<QImage> m_frameQueue;
    int m_maxSize;
    mutable QMutex m_mutex;
    QWaitCondition m_bufferNotFull;
    QWaitCondition m_bufferNotEmpty;
};

#endif // FRAMEBUFFER_H
