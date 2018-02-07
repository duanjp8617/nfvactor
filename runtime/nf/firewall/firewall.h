#ifndef FIREWALL_H
#define FIREWALL_H

#include "../../bessport/packet.h"
#include "firewall_state.h"
#include "../base/flow_al_status.h"

#include <glog/logging.h>

class firewall{
public:
  inline bool nf_logic_impl(bess::Packet* pkt, firewall_fs* fs, firewall_shared_state* ss){
    bool result = true;

    // first perform basic
    switch(fs->connection_status){
      case no_conneciton : {
        //LOG(INFO)<<"The first packet of the flow is received.";
        // we have done packet checking in the actor processing
        // pipeline, there is no need to perform any kind of
        // packet checking here.
        uint8_t protocol_type = *(pkt->head_data<uint8_t*>(14+9));
        if(protocol_type!=protocol_tcp && protocol_type!=protocol_udp){
          //LOG(INFO)<<"The first packet is not udp or tcp, go to bad connection.";
          // we only accept tcp and udp.
          fs->connection_status = bad_connection;
          fs->dropped_packets += 1;
          result = false;
          break;
        }

        //LOG(INFO)<<"Saving protocol type, src/dst ip/port";

        fs->protocol = protocol_type;
        fs->src_port = *(pkt->head_data<uint16_t*>(14+20));
        fs->dst_port = *(pkt->head_data<uint16_t*>(14+20+2));

        // transform the stored src/dst ip to local representation
        // so that we can correct compare the ip with the firewall rule
        fs->src_ip = ntol(*(pkt->head_data<uint32_t*>(14+12)));
        fs->dst_ip = ntol(*(pkt->head_data<uint32_t*>(14+16)));

        /*if( (fs->protocol == protocol_tcp) &&
            ( ((*(pkt->head_data<uint8_t*>(14+20+13)))&0x02) != 0x02) ){
          // this is a tcp packet, but the first packet is not
          // a syn packet.
          fs->connection_status = bad_connection;
          fs->dropped_packets += 1;
          result = false;
          break;
        }*/

        if(ss->match(fs->src_ip, fs->dst_ip)){
          //LOG(INFO)<<"The flow matches the rule, go to bad connection.";
          // the firewall rules prevent the connection from getting established
          fs->connection_status = bad_connection;
          fs->dropped_packets += 1;
          result = false;
          break;
        }

        //LOG(INFO)<<"The flow does not matches the rule, go to connection up.";
        fs->connection_status = connection_up;
        fs->passed_pkts += 1;
        return true;
        break;
      }
      case connection_up : {
        if( (fs->protocol == protocol_tcp) &&
            ( ((*(pkt->head_data<uint8_t*>(14+20+13)))&0x01) == 0x01) ){
          //LOG(INFO)<<"The flow sees the last packet. The connection is down";
          // this is a tcp fin packet, the connection is down.
          fs->connection_status = connection_down;
        }

        //LOG(INFO)<<"The flow receives new flow packet.";
        fs->passed_pkts += 1;
        return true;
        break;
      }
      case connection_down : {
        // if we receive any other packets when the connection is down
        // we drop the packet and transitions into bad_connection
        //LOG(INFO)<<"New packet after the connection is down. Go to bad connection.";
        fs->connection_status = bad_connection;
        fs->dropped_packets += 1;
        result = false;
        break;
      }
      case bad_connection : {
        //LOG(INFO)<<"The connection is bad, drop the packet.";
        fs->dropped_packets += 1;
        result = false;
        break;
      }
      default:
        break;
    }

    return result;
  }

  inline void nf_flow_arrive_impl(firewall_fs* fs, firewall_shared_state* ss, flow_arrive_status status){
    //LOG(INFO)<<"flow arrive, do nothing";
  }

  inline void nf_flow_leave_impl(firewall_fs* fs, firewall_shared_state* ss, flow_leave_status status){
    //LOG(INFO)<<"flow leaves, do nothing";
  }

  inline void nf_init_fs_impl(firewall_fs* fs){
    //LOG(INFO)<<"New flow comes, initiate firewall flow state";
    fs->connection_status = no_conneciton;
  }

private:
  uint32_t ntol(uint32_t network_representation){
    char buf[4];
    char* ptr = (char*)(&network_representation);

    buf[3] = ptr[0];
    buf[2] = ptr[1];
    buf[1] = ptr[2];
    buf[0] = ptr[3];

    uint32_t local_representation = *((uint32_t*)buf);
    return local_representation;
  }

};

#endif
