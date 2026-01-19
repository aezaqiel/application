#pragma once

#include "core/window.hpp"
#include "core/events.hpp"

namespace application {

    class Renderer
    {
    public:
        Renderer(const Window& window, EventDispatcher& dispatcher);
        ~Renderer();

        void draw();

    private:
    };

}
