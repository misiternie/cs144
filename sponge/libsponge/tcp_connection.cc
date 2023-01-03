// #include "tcp_connection.hh"

// #include <iostream>

// // Dummy implementation of a TCP connection

// // For Lab 4, please replace with a real implementation that passes the
// // automated checks run by `make check`.

// template <typename... Targs>
// void DUMMY_CODE(Targs &&... /* unused */) {}

// using namespace std;

// //add code

// size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

// size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

// size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

// size_t TCPConnection::time_since_last_segment_received() const { return _ms_since_last_segment_received; }

// // void TCPConnection::segment_received(const TCPSegment &seg) {
// //     if (!_active)//连接已关闭
// //         return;
// //     _ms_since_last_segment_received = 0;

// //     // data segments with acceptable ACKs should be ignored in SYN_SENT
// //     if (in_syn_sent() && seg.header().ack && seg.payload().size() > 0) {
// //         return;
// //     }
// //     bool send_empty = false;
// //     if (_sender.next_seqno_absolute() > 0 && seg.header().ack) {
// //         // unacceptable ACKs should produced a segment that existed
// //         if (!_sender.ack_received(seg.header().ackno, seg.header().win)) {
// //             send_empty = true;
// //         }
// //     }

// //     bool recv_flag = _receiver.segment_received(seg);
// //     if (!recv_flag) {
// //         send_empty = true;
// //     }

// //     if (seg.header().syn && _sender.next_seqno_absolute() == 0) {
// //         connect();
// //         return;
// //     }

// //     if (seg.header().rst) {
// //         // RST segments without ACKs should be ignored in SYN_SENT
// //         if (in_syn_sent() && !seg.header().ack) {
// //             return;
// //         }
// //         unclean_shutdown(false);
// //         return;
// //     }

// //     if (seg.length_in_sequence_space() > 0) {
// //         send_empty = true;
// //     }

// //     if (send_empty) {
// //         if (_receiver.ackno().has_value() && _sender.segments_out().empty()) {
// //             _sender.send_empty_segment();
// //         }
// //     }
// //     push_segments_out();
// // }

// bool TCPConnection::active() const { return _active; }

// size_t TCPConnection::write(const string &data) {
//     size_t ret = _sender.stream_in().write(data);
//     push_segments_out();
//     return ret;
// }

// //! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
// void TCPConnection::tick(const size_t ms_since_last_tick) {
//     if (!_active)
//         return;
//     _ms_since_last_segment_received += ms_since_last_tick;
//     _sender.tick(ms_since_last_tick);
//     if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
//         unclean_shutdown(true);
//     }
//     push_segments_out();
// }

// void TCPConnection::end_input_stream() {
//     _sender.stream_in().end_input();
//     push_segments_out();
// }

// void TCPConnection::connect() {
//     // when connect, must active send a SYN
//     push_segments_out();
// }

// TCPConnection::~TCPConnection() {
//     try {
//         if (active()) {
//             // Your code here: need to send a RST segment to the peer
//             cerr << "Warning: Unclean shutdown of TCPConnection\n";
//             unclean_shutdown(true);
//         }
//     } catch (const exception &e) {
//         std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
//     }
// }

// bool TCPConnection::push_segments_out() {
//     // default not send syn before recv a SYN
//     _sender.fill_window();
//     TCPSegment seg;
//     while (!_sender.segments_out().empty()) {
//         seg = _sender.segments_out().front();
//         _sender.segments_out().pop();
//         if (_receiver.ackno().has_value()) {
//             seg.header().ack = true;
//             seg.header().ackno = _receiver.ackno().value();
//             seg.header().win = _receiver.window_size();
//         }
//         if (_need_send_rst) {
//             _need_send_rst = false;
//             seg.header().rst = true;
//         }
//         _segments_out.push(seg);
//     }
//     clean_shutdown();
//     return true;
// }

// void TCPConnection::unclean_shutdown(bool send_rst) {
//     _receiver.stream_out().set_error();
//     _sender.stream_in().set_error();
//     _active = false;
//     if (send_rst) {
//         _need_send_rst = true;
//         if (_sender.segments_out().empty()) {
//             _sender.send_empty_segment();
//         }
//         push_segments_out();
//     }
// }

// bool TCPConnection::clean_shutdown() {
//     if (_receiver.stream_out().input_ended() && !(_sender.stream_in().eof())) {
//         _linger_after_streams_finish = false;
//     }
//     if (_sender.stream_in().eof() && _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) {
//         if (!_linger_after_streams_finish || time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
//             _active = false;
//         }
//     }
//     return !_active;
// }

// bool TCPConnection::in_listen() { return !_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0; }

// bool TCPConnection::in_syn_recv() { return _receiver.ackno().has_value() && !_receiver.stream_out().input_ended(); }

// bool TCPConnection::in_syn_sent() {
//     return _sender.next_seqno_absolute() > 0 && _sender.bytes_in_flight() == _sender.next_seqno_absolute();
// }

// void TCPConnection::segment_received(const TCPSegment &seg) {
//     if (!_active) {
//         return;
//     }
 
//     _ms_since_last_segment_received = 0;
 
//     if (seg.header().rst) {
//         unclean_shutdown(false);
//         return;
//     }
 
//     // closed
//     if (_sender.next_seqno_absolute() == 0) {
//         // passive open
//         if (seg.header().syn) {
//             // todo Don't have ack no, so sender don't need ack
//             _receiver.segment_received(seg);
//             connect();
//         }
//         return;
//     }
 
//     // syn-sent
//     if (_sender.next_seqno_absolute() == _sender.bytes_in_flight() && !_receiver.ackno().has_value()) {
//         if (seg.header().syn && seg.header().ack) {
//             // active open
//             _sender.ack_received(seg.header().ackno, seg.header().win);
//             _receiver.segment_received(seg);
//             // send ack
//             _sender.send_empty_segment();
//             push_segments_out();
//             // become established
            
//         } else if (seg.header().syn && !seg.header().ack) {
//             // simultaneous open
//             _receiver.segment_received(seg);
//             // already send syn, need a ack
//             _sender.send_empty_segment();
//             push_segments_out();
//             // become syn_rcvd
//         }
//         return;
//     }
 
//     // syn-rcvd
//     if (_receiver.ackno().has_value() && !_receiver.stream_out().input_ended() &&
//         _sender.next_seqno_absolute() == _sender.bytes_in_flight()) {
//         // receive ack
//         // todo need ack
//         _receiver.segment_received(seg);
//         _sender.ack_received(seg.header().ackno, seg.header().win);
//         return;
//     }
 
//     // established, aka stream ongoing
//     if (_sender.next_seqno_absolute() > _sender.bytes_in_flight() && !_sender.stream_in().eof()) {
//         _sender.ack_received(seg.header().ackno, seg.header().win);
//         _receiver.segment_received(seg);
//         if (seg.length_in_sequence_space() > 0) {
//             _sender.send_empty_segment();
//         }
//         //_sender.fill_window();
//         push_segments_out();
//         return;
//     }
 
//     // close wait
//     if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended()) {
//         _sender.ack_received(seg.header().ackno, seg.header().win);
//         _receiver.segment_received(seg);
//         // try to send remain data
//         //_sender.fill_window();
//         push_segments_out();
//         return;
//     }
 
//     // FIN_WAIT_1
//     if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
//         _sender.bytes_in_flight() > 0 && !_receiver.stream_out().input_ended()) {
//         if (seg.header().fin && seg.header().ack) {
//             _sender.ack_received(seg.header().ackno, seg.header().win);
//             _receiver.segment_received(seg);
//             _sender.send_empty_segment();
//             push_segments_out();
//         } else if (seg.header().fin && !seg.header().ack) {
//             _sender.ack_received(seg.header().ackno, seg.header().win);
//             _receiver.segment_received(seg);
//             _sender.send_empty_segment();
//             push_segments_out();
//         } else if (!seg.header().fin && seg.header().ack) {
//             _sender.ack_received(seg.header().ackno, seg.header().win);
//             _receiver.segment_received(seg);
//             push_segments_out();
//         }
 
//         return;
//     }
 
//     // CLOSING
//     if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
//         _sender.bytes_in_flight() > 0 && _receiver.stream_out().input_ended()) {
//         _sender.ack_received(seg.header().ackno, seg.header().win);
//         _receiver.segment_received(seg);
//         push_segments_out();
//         return;
//     }
 
//     // FIN_WAIT_2
//     if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
//         _sender.bytes_in_flight() == 0 && !_receiver.stream_out().input_ended()) {
//         _sender.ack_received(seg.header().ackno, seg.header().win);
//         _receiver.segment_received(seg);
//         _sender.send_empty_segment();
//         push_segments_out();
//         return;
//     }
 
//     // TIME_WAIT
//     if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
//         _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended()) {
//         if (seg.header().fin) {
//             _sender.ack_received(seg.header().ackno, seg.header().win);
//             _receiver.segment_received(seg);
//             _sender.send_empty_segment();
//             push_segments_out();
//         }
 
//         return;
//     }
//         // 有些状态没有预判到，这里统一处理下。
//     _sender.ack_received(seg.header().ackno, seg.header().win);
//     _receiver.segment_received(seg);
//     //_sender.fill_window();
//     push_segments_out();
// }

#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!_active) {
        return;
    }
    // reset the timer
    _time_since_last_segment_received = 0;
    const TCPHeader &header = seg.header();
    // if TCP does not receive SYN from remote peer, and not send SYN to remote peer
    if (!_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0) {
        // at this time, TCP acts as a server,
        // and should not receive any segment except it has SYN flag
        if (!header.syn) {
            return;
        }
        _receiver.segment_received(seg);
        // try to send SYN segment, use for opening TCP at the same time
        connect();
        return;
    }
    // the TCP ends in an unclean shutdown when receiving RST segment
    if (header.rst) {
        unclean_shutdown();
        return;
    }
    _receiver.segment_received(seg);
    // if TCP sends SYN segment as a client but does not receive SYN from remote peer,
    // the local TCP should not handle it, too.
    if (!_receiver.ackno().has_value()) {
        return;
    }

    // set the `_linger_after_streams_finish` the first time the inbound stream ends
    if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended()) {
        _linger_after_streams_finish = false;
    }
    // use the remote peer's ackno and window size to update the local sending window
    if (header.ack) {
        _sender.ack_received(header.ackno, header.win);
    }

    _sender.fill_window();
    // makes sure that at least one segment is sent in reply
    if (seg.length_in_sequence_space() > 0 && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    // an extra special case to respond to a keep-alive segment
    if (seg.length_in_sequence_space() == 0 && header.seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }
    send_segments();
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    if (!_active) {
        return 0;
    }
    size_t ret = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return ret;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!_active) {
        return;
    }
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    // TCP unclean shutdown if the number of consecutive retransmissions
    // is more than an upper limit
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        send_RST();
        return;
    }
    // TCP clean shutdown if necessary
    if (_receiver.stream_out().input_ended() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0 &&
        (!_linger_after_streams_finish || _time_since_last_segment_received >= 10 * _cfg.rt_timeout)) {
        _active = false;
        return;
    }
    // send segments when `_sender.tick()` has a retransmission
    send_segments();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_segments();
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            send_RST();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::send_segments() {
    while (!_sender.segments_out().empty()) {
        TCPSegment segment = _sender.segments_out().front();
        _sender.segments_out().pop();
        optional<WrappingInt32> ackno = _receiver.ackno();
        // if TCP does not receive SYN segments from the remote peer, i.e. at SYN_SENT state
        // TCP will not set ACK flag and seqno
        if (ackno.has_value()) {
            segment.header().ack = true;
            segment.header().ackno = ackno.value();
        }
        // set the local receiver's window size
        segment.header().win = _receiver.window_size();
        _segments_out.emplace(segment);
    }
}

void TCPConnection::send_RST() {
    _sender.fill_window();
    if (_sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    TCPSegment segment = _sender.segments_out().front();
    _sender.segments_out().pop();
    optional<WrappingInt32> ackno = _receiver.ackno();
    if (ackno.has_value()) {
        segment.header().ack = true;
        segment.header().ackno = ackno.value();
    }
    segment.header().win = _receiver.window_size();
    segment.header().rst = true;
    _segments_out.emplace(segment);

    unclean_shutdown();
}

inline void TCPConnection::unclean_shutdown() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}
