#include <net/arp.h>

using namespace common;
using namespace net;
using namespace drivers;

AddressResolutionProtocol::AddressResolutionProtocol(EtherFrameProvider *backend)
: EtherFrameHandler(backend, 0x806)
{
    numCacheEntries = 0;
}
AddressResolutionProtocol::~AddressResolutionProtocol() {

}

bool AddressResolutionProtocol::OnEtherFrameReceived(uint8_t *etherframePayload, uint32_t size) {
    if(size < sizeof(AddressResolutionProtocolMessage)){
        return false;
    }
    AddressResolutionProtocolMessage* arp = (AddressResolutionProtocolMessage*) etherframePayload;
    if(arp->hardwareType == 0x0100){
        if(arp->protocol == 0x0008
        && arp->hardwareAddressSize == 6
        && arp->protocolAddressSize == 4
        && arp->dstIP == backend->GetIPAddress()){
            switch (arp->command){
                case 0x0100:
                    arp->command = 0x0200;
                    arp->dstIP = arp->srcIP;
                    arp->dstMAC = arp->srcMAC;
                    arp->srcIP = backend->GetIPAddress();
                    arp->srcMAC = backend->GetMACAddress();
                    return true;
                    break;
                case 0x0200:
                    if(numCacheEntries < 128){
                        IPcache[numCacheEntries] = arp->srcIP;
                        MACcache[numCacheEntries] = arp->srcMAC;
                        numCacheEntries++;
                    }
                    break;

            }
        }
    }
    return false;
}

void AddressResolutionProtocol::RequestMACAddress(common::uint32_t IP_BE) {

    AddressResolutionProtocolMessage arp;
    arp.hardwareType = 0x0100;
    arp.protocol = 0x0008;
    arp.hardwareAddressSize = 6;
    arp.protocolAddressSize = 4;
    arp.command = 0x0100;

    arp.srcMAC = backend->GetMACAddress();
    arp.srcIP = backend->GetIPAddress();
    arp.dstMAC = 0xFFFFFFFFFFFF;
    arp.dstIP = IP_BE;

    this->Send(arp.dstMAC, (uint8_t*)&arp, sizeof(AddressResolutionProtocolMessage));
}

uint64_t AddressResolutionProtocol::GetMACFromCache(common::uint32_t IP_BE) {
    for(int i = 0; i < numCacheEntries; i++){
        if(IPcache[i] == IP_BE){
            return MACcache[i];
        }
    }
    return 0xFFFFFFFFFFFF;
}
uint64_t AddressResolutionProtocol::Resolve(common::uint32_t IP_BE) {
    uint64_t result = GetMACFromCache(IP_BE);
    if(result == 0xFFFFFFFFFFFF){
        RequestMACAddress(IP_BE);
    }
    while(result == 0xFFFFFFFFFFFF){
        result = GetMACFromCache(IP_BE);
    }
    return result;
}