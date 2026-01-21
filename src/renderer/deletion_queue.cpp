#include "deletion_queue.hpp"

namespace application {

    DeletionQueue::~DeletionQueue()
    {
        flush();
    }

    void DeletionQueue::push(std::function<void()> deletor)
    {
        m_queue.push_back(std::forward<Deletor>(deletor));
    }

    void DeletionQueue::flush()
    {
        for (auto& deletor : (m_queue | std::ranges::views::reverse)) {
            std::invoke(std::forward<Deletor>(deletor));
        }

        m_queue.clear();
    }

}
