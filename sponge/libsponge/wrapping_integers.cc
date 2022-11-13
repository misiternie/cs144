#include "wrapping_integers.hh"
#include <iostream>
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {

    return WrappingInt32{static_cast<uint32_t>(n) + isn.raw_value()}; 
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {

    uint32_t offset = n.raw_value() - isn.raw_value();
    uint64_t ans = (checkpoint & 0xffffffff00000000) + offset;  //0xffffffff00000000 保留前八位，因为都是2^32的倍数
    uint64_t temp = ans;

    //分3种情况
    //1: 如果加一个2^32距离checkpoint更近
    if (abs(int64_t(temp + (1ul << 32) - checkpoint)) < abs(int64_t(temp - checkpoint)))
        ans = temp + (1ul << 32);

    //2：如果减一个2^32距离checkpoint更近
    if (temp >= (1ul << 32) && abs(int64_t(temp - (1ul << 32) - checkpoint)) < abs(int64_t(temp - checkpoint)))
        ans = temp - (1ul << 32);
    //3：不做修改

    return ans;

}
