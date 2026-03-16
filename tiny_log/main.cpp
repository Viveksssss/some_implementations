#include "tiny_log.hpp"

using namespace tinylog;

int main()
{
    set_log_file("log.txt");
    log_debug("hello {}", 25);
    log_info("hello {}", 25);
}
