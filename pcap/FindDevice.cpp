#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pcap.h>

std::ostream& operator << (std::ostream& os, pcap_addr_t& addr) {
	return os;
}

std::ostream& operator << (std::ostream& os, pcap_if_t& dev) {
	os << dev.name << ": "
		<< (dev.description ? dev.description : "");
	for (pcap_addr_t* addrp = dev.addresses; addrp; addrp = addrp->next) {
		auto in = reinterpret_cast<struct sockaddr_in*>(addrp->addr);
		os << "addr: " << inet_ntoa(in->sin_addr) << ' ';
		if (addrp->dstaddr) {
			in = reinterpret_cast<struct sockaddr_in*>(addrp->dstaddr);
			os << "dstaddr: " << inet_ntoa(in->sin_addr) << ' ';
		}
	}
	return os;
}

int main() {
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_if_t* devicesp{};

	int rc = pcap_findalldevs(&devicesp, errbuf);
	if (rc) {
		std::cout << "Error finding device " << errbuf << std::endl;
		return 1;
	}

	for (pcap_if_t* devp = devicesp; devp; devp = devp->next) {
		std::cout << *devp << std::endl;
	}
	pcap_freealldevs(devicesp);
	return 0;
}
