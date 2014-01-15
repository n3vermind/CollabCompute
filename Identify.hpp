#include <string>
#include <cstdlib>
#include <ctime>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "sha1.h"

class Identify {
    public:
        static std::string getId();
};
