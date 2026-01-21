#pragma once

namespace application {

    class DeletionQueue
    {
        using Deletor = std::function<void()>;
    public:
        DeletionQueue() = default;
        ~DeletionQueue();

        void push(Deletor deletor);
        void flush();

    private:
        std::vector<Deletor> m_queue;
    };

}
