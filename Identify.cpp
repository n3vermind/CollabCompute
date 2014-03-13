#include "Identify.hpp"

std::string Identify::getId()
{
    int s, len;
    ifreq *ifr;
    ifconf ifc;
    char hex[41], buffer[1024];
    unsigned char hash[20];

/*    s = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = buffer;
    ioctl(s, SIOCGIFCONF, &ifc);

    ifr = ifc.ifc_req;
    len = ifc.ifc_len / sizeof(ifreq);

    for(int i=0; i<len; ++i) {
        ioctl(s, SIOCGIFHWADDR, &ifr[i]);
        if(ifr[i].ifr_hwaddr.sa_data[0] != 0) {
            sha1::calc(ifr[i].ifr_hwaddr.sa_data, 6, hash);
            sha1::toHexString(hash, hex);
            return std::string(hex);
        }
    }
*/
    srand(time(NULL));
    std::string id = std::to_string(rand());
    sha1::calc(id.c_str(), id.length(), hash);
    sha1::toHexString(hash, hex);
    return std::string(hex);
}
