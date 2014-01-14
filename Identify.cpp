#include "Identify.hpp"

std::string Identify::getId()
{
    ifreq ifr;
    char hex[41];
    unsigned char hash[20];

    strcpy(ifr.ifr_name, "eth0");
    ioctl(socket(AF_INET, SOCK_DGRAM, 0), SIOCGIFHWADDR, &ifr);
    sha1::calc(ifr.ifr_hwaddr.sa_data, 6, hash);
    sha1::toHexString(hash, hex);
    return std::string(hex);
}
