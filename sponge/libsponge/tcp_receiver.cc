#include "tcp_receiver.hh"
#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {

    TCPHeader const &header = seg.header(); 
    Buffer const &data = seg.payload();
    bool syn = header.syn, fin = header.fin;
    WrappingInt32 seqno = header.seqno;


    if (!_recv_syn && syn) {    //首次收到syn
        _isn = seqno;
        seqno = seqno + 1;  //第一次加1， syn占一个字节
        _recv_syn = true;
    }

    if (_recv_syn) {
        _reassembler.push_substring(data.copy(), unwrap(seqno-1, _isn, _checkpoint), fin);  //减1去掉syn
        _checkpoint = _reassembler.get_reassemble_start();
    }

    
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if (!_recv_syn) return nullopt;
    return wrap(_checkpoint + 1 + _reassembler.stream_out().input_ended(), _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
