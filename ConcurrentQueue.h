#ifndef _CONCURRENT_QUEUE_H_
#define _CONCURRENT_QUEUE_H_
#include <malloc.h>
#include <memory>

#ifdef _X86_
typedef unsigned int usize_t;
#define CompareAndExchange CompareAndExchange8
#define CMPXCHG_ALIGNED_SIZE 4

#define NEW_NODE(name) \
    Node* name = new Node();

#else
#ifdef _AMD64_
typedef unsigned __int64 usize_t;

extern "C" bool CompareAndExchange16(volatile void* toXch, volatile void* ptr11,
    usize_t cnt1, volatile void* ptr12, usize_t cnt2);
#define CMPXCHG_ALIGNED_SIZE 16
#define CompareAndExchange CompareAndExchange16

#define NEW_NODE(name) \
    Node* name = (Node*)_aligned_malloc(sizeof(ConcurrentQueue), CMPXCHG_ALIGNED_SIZE); \
    new(name)Node();

#endif
#endif

template<class T>
class ConcurrentQueue
{
private:
    struct Node;
    struct Pointer
    {
        volatile Node* ptr1;
        volatile usize_t cnt;
        bool operator==(volatile Pointer& p2)
        {
            return this->ptr1 == p2.ptr1 && this->cnt == p2.cnt;
        }
        Pointer& operator=(volatile Pointer& p2)
        {
            this->ptr1 = p2.ptr1;
            this->cnt = p2.cnt;
            return *this;
        }
    };
    struct Node
    {
        Pointer next;
        T data;
    };

    volatile Pointer head_;
    volatile Pointer tail_;

#ifdef _X86_
    static inline bool CompareAndExchange8(volatile Pointer* toXch, volatile Node* ptr11,
        usize_t cnt1, volatile Node* ptr12, usize_t cnt2)
    {
        __asm
        {
            push ebx
            push esi
            mov ebx, ptr12
            mov ecx, cnt2
            mov eax, ptr11
            mov edx, cnt1
            mov esi, toXch
            lock cmpxchg8b[esi]
            sete al
            pop esi
            pop ebx
        }
    }
#endif

public:
    /*Only need when in amd64 mode*/
    static ConcurrentQueue* CreateConcurrentQueuePointer()
    {
        ConcurrentQueue* q = (ConcurrentQueue*)_aligned_malloc(sizeof(ConcurrentQueue), CMPXCHG_ALIGNED_SIZE);
        if (!q)
            return q;
        new (q)ConcurrentQueue();
        return q;
    }

    static void DeleteConcurrentQueue(ConcurrentQueue* q)
    {
        q->~ConcurrentQueue();
        _aligned_free(q);
    }

#if _MSC_VER >= 1600 //vs2010, c++11 supported
    static std::shared_ptr<ConcurrentQueue> CreateConcurrentQueue()
    {
        ConcurrentQueue* q = (ConcurrentQueue*)_aligned_malloc(sizeof(ConcurrentQueue), CMPXCHG_ALIGNED_SIZE);
        if (!q)
            return q;
        new (q)ConcurrentQueue();
        return std::shared_ptr<ConcurrentQueue>(q, [](ConcurrentQueue* p){q->~ConcurrentQueue(); _aligned_free(p); });
    }
#endif


    ConcurrentQueue()
    {
        NEW_NODE(n);
        n->next.ptr1 = 0;
        head_.ptr1 = tail_.ptr1 = n;
        head_.cnt = tail_.cnt = n->next.cnt = 0;
    }

    ~ConcurrentQueue()
    {
        T temp;
        while (Dequeue(temp))
            ;
        if (head_.ptr1)
            delete head_.ptr1;
    }

    void Enqueue(T& data)
    {
        NEW_NODE(node);
        node->data = data;
        node->next.ptr1 = 0;

        Pointer t, n;
        while (true)
        {
            t = tail_;
            n = t.ptr1->next;
            if (t == tail_)
            {
                if (n.ptr1 == 0)
                {
                    if (CompareAndExchange(&t.ptr1->next, n.ptr1, n.cnt, node, n.cnt + 1))
                        break;
                }
                else
                {
                    CompareAndExchange(&tail_, t.ptr1, t.cnt, n.ptr1, t.cnt + 1);
                }
            }
        }
        CompareAndExchange(&tail_, t.ptr1, t.cnt, node, t.cnt + 1);
    }

    bool Dequeue(T& data)
    {
        Pointer h, t, n;
        while (true)
        {
            h = head_;
            t = tail_;
            n = h.ptr1->next;
            if (h == head_)
            {
                if (h.ptr1 == t.ptr1)
                {
                    if (n.ptr1 == 0)
                        return false;
                    CompareAndExchange(&tail_, t.ptr1, t.cnt, n.ptr1, t.cnt + 1);
                }
                else
                {
                    data = *(T*)&n.ptr1->data;
                    if (CompareAndExchange(&head_, h.ptr1, h.cnt, n.ptr1, h.cnt + 1))
                        break;
                }
            }
        }
        delete h.ptr1;
        return true;
    }
};

#endif