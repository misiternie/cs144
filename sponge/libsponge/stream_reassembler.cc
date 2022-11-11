#include "stream_reassembler.hh"
#include <iostream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    size_t beg  = max(index, _first_unassembled), end = index + data.size();

    if (end <= _first_unassembled) {    //重复字符串
        if (eof && empty())    _output.end_input(); //遇到空字符串，eof = 1时的情况
        return ;
    }

    if (end > _output.bytes_read() + _capacity)
        end = _output.bytes_read() + _capacity;
    else if (eof)
        _eof = eof; //读到了末尾的eof
    


    size_t n = _data_set.size(), i = beg;
    for (auto it = data.begin() + beg - index; it != data.begin() + end - index; it ++)
        _data_set.insert({i ++, *it});

    _accepted_len += _data_set.size() - n;

    if (!_data_set.empty()) {
        auto it = _data_set.begin();
        string temp = "";
        for ( ; it != _data_set.end() && it->first == _first_unassembled; it ++)
            temp.push_back(it->second), _first_unassembled ++; 
        _output.write(temp);
        _data_set.erase(_data_set.begin(), it);
    }

    if (_eof && empty())
        _output.end_input(), _eof = false;

}

size_t StreamReassembler::unassembled_bytes() const { return _accepted_len - _first_unassembled; }

bool StreamReassembler::empty() const { return _data_set.empty(); }

size_t StreamReassembler::stream_reassembler_size() const { return _capacity - _output.buffer_size() - unassembled_bytes(); }
