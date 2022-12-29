#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include<iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {

    TCPSegment seg;
    //如果还未发送syn
    if (_next_seqno == 0) {
        seg.header().syn = true;
        send_not_empty_segment(seg);
        return ;
    }
    else if (bytes_in_flight() == _next_seqno)
        return ;


    uint64_t window_size = _window_size ? _window_size : 1;
    uint64_t remain_bytes;

    while ((remain_bytes = window_size - (_next_seqno - _ackno))) { //window_size包括已发送但是未确认的序列，所以需要减去他们
        //std::cout << remain_bytes << "  "<< window_size << "\r\n";
        //std::cout << "the _bytes_in_flight is : " << _bytes_in_flight << "   the _next_seqno is : " << _next_seqno << "  the _ackno is :  " << _ackno << "\r\n";
        uint64_t len = TCPConfig::MAX_PAYLOAD_SIZE > remain_bytes ? remain_bytes : TCPConfig::MAX_PAYLOAD_SIZE;

        if (_stream.eof()) {

            if (_next_seqno < _stream.bytes_written() + 2) {    //fin没有发送
                seg.header().fin = true;
                send_not_empty_segment(seg);
            }
            else return;

        }

        else {
            seg.payload() = Buffer(std::move(_stream.read(len)));

            if (_stream.eof() && remain_bytes - seg.length_in_sequence_space() > 0)
                seg.header().fin = true;
            if (seg.length_in_sequence_space() == 0)    
                return ;
            send_not_empty_segment(seg);
        }
        
    }
    
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    uint64_t new_ackno = unwrap(ackno, _isn, _ackno);
    //std::cout << ackno.raw_value() << "   " << new_ackno << "\r\n";
    if (new_ackno > _next_seqno)  //不在正常区间的ack直接丢弃
        return ;

    _window_size = static_cast<size_t>(window_size);//必须放在下面那个判断之前，因为即使是后来的ack，它的winsize也是合法的

    if (new_ackno <= _ackno)
        return ;

    _ackno = new_ackno;

    _timer.start(); //restart

    while (!_segments_out_temp.empty()) {  //丢弃已经完全确认的段，不再需要追踪
        TCPSegment &seg = _segments_out_temp.front();
        if (unwrap(seg.header().seqno, _isn, _ackno) + seg.length_in_sequence_space() > _ackno)
        // if (ackno.raw_value() < seg.header().seqno.raw_value() 
        // + static_cast<uint32_t>(seg.length_in_sequence_space()))
            break;
        _segments_out_temp.pop();
        _bytes_in_flight -= seg.length_in_sequence_space();
    }
    //std::cout << _next_seqno << " _bytes_in_flight :  " << _bytes_in_flight << "\r\n";
    fill_window();

    if (_segments_out_temp.empty())
        _timer.close();

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    if (!_timer.running() || !_timer.timeout(ms_since_last_tick))
        return ;
    
    if (_segments_out_temp.empty()) {
        _timer.close();
        return ;
    }

    _timer.doubleOrkeep_RTO_and_restart(_window_size);  //更新计时器
    _segments_out.push(_segments_out_temp.front()); //重传最早的段
    
}

unsigned int TCPSender::consecutive_retransmissions() const { return _timer.count_of_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}

void TCPSender::send_not_empty_segment(TCPSegment &seg) {

    seg.header().seqno = wrap(_next_seqno, _isn);
    
    // std::cout << "here send a segment, and the _bytes_in_flight is : " << _bytes_in_flight << "   the _next_seqno is : " << _next_seqno << "  the _ackno is :  " << _ackno << "\r\n";
    _segments_out.push(seg);
    _segments_out_temp.push(seg);
    _bytes_in_flight += seg.length_in_sequence_space();
    _next_seqno += seg.length_in_sequence_space();

    if (!_timer.running())
        _timer.start();

}
