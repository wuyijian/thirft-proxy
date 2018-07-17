#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include "net_utils.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sstream>


using std::string;

namespace Common
{
	string local_path = " ";
	string local_ip = " ";
    // 没有获取到则返回空字符串
    string get_local_ip()
    {
        int sock_get_ip;
        struct sockaddr_in *sin;
        struct ifreq    ifr_ip;
    
        if((sock_get_ip = socket(AF_INET,SOCK_STREAM,0)) == -1){
            ERROR_LOG("get local ip err");
            return "";
        }
    
        const char* if_list[] = {"eth0", "eth1", "eth2", "eth3"};
    
        for(unsigned i = 0; i < sizeof(if_list)/sizeof(const char*); ++i)
        {
            memset(&ifr_ip,0,sizeof(ifr_ip));
            strncpy(ifr_ip.ifr_name, if_list[i], sizeof(ifr_ip.ifr_name)-1);
            if(ioctl(sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0)
            {
                continue;
            }

            sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    
            string ip = inet_ntoa(sin->sin_addr);
    
            if (ip.find("192.168") == 0 || ip.find("10.") == 0 || ip.find("172.") == 0) {
//                DEBUG_LOG("local ip: %s", ip.c_str());
//              
    			close(sock_get_ip);
                return ip;
            }
        }
		close(sock_get_ip);
        return "";
    }

	bool get_ip_by_host(string host, vector<string> &ips)
	{
	//通过域名获取IP地址
		char **pptr;
		struct hostent *hptr;
		if((hptr = gethostbyname(host.c_str())) == NULL){
			ERROR_LOG("Get service center ip failed.");
			return false;
		}

		char   str[32];
		switch(hptr->h_addrtype)
    	{
    	    case AF_INET:
    	    case AF_INET6:
				pptr=hptr->h_addr_list;
				for(; *pptr!=NULL; pptr++){
					ips.push_back(inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
				}
    	    break;
    	    default:
    	      ERROR_LOG("unknown address type.");
		}
		return true;
	}

    vector<std::pair<string, int> > string_to_ip_port_pairs(string addrs)
    {
        std::istringstream is(addrs);
        string addr;
        vector<std::pair<string, int> > ip_port_pairs;
        while (getline(is, addr, ';'))
        {
            /// addr 进一步拆分
            string::size_type n = addr.find(':');
            if (n == string::npos) {
                ERROR_LOG("string_to_ip_port_pairs failed, config format error");
            } else {
                DEBUG_LOG("addr %s", addr.c_str());
                string ip = addr.substr(0, n);
                int port = atoi(addr.substr(n + 1).c_str());
                ip_port_pairs.push_back(std::make_pair(ip, port));
            }
        }
        return ip_port_pairs;
    }
};
