#include "core/application.hpp"

int main()
{
    auto app = std::make_unique<application::Application>();
    app->run();
}
