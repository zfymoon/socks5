#include "server.h"

int main() {
    server proxy_server(9981);
    proxy_server.run();
    return 0;
}
