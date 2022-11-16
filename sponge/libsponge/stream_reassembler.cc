#include "stream_reassembler.hh"
#include <iostream>


//add include
#include <algorithm>
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

    //beg：待插入set的string的起始索引。end : 待插入set的string下一位的末尾索引
    size_t beg  = max(index, _first_unassembled), end = index + data.size();
    
    if (end <= _first_unassembled) {    //重复字符串或空串
        if (eof && empty())    _output.end_input(); //遇到空字符串，且eof = 1时的情况
        return ;
    }

    if (end > _output.bytes_read() + _capacity) //如果超出capacity，截断
        end = _output.bytes_read() + _capacity;
    else if (eof)   
        _eof = eof; //读到了末尾的eof
    
    while (beg < end)
        //merge,除掉重复部分并插入_data_set
        beg = merge(beg, end, index, data);

    //更新已存储字符串，组建好连续的就直接写入ByteStream
    if (!empty()) {
        auto it = _data_set.begin();
        for ( ; it != _data_set.end() && it->first == _first_unassembled; it ++)
            _first_unassembled += it->second.size(), _output.write(it->second);
            
        _data_set.erase(_data_set.begin(), it);
    }
    // std::cout << "\r\n";


    if (_eof && empty())
        _output.end_input(), _eof = false;

}

size_t StreamReassembler::unassembled_bytes() const { return _accepted_len - _first_unassembled; }

bool StreamReassembler::empty() const { return _data_set.empty(); }

size_t StreamReassembler::stream_reassembler_size() const { return _capacity - _output.buffer_size() - unassembled_bytes(); }

size_t StreamReassembler::merge(size_t beg, size_t end, const size_t index, const string &data) {

    if (_data_set.empty()) {    //_data_set为空，整个插入
        _data_set.insert({beg, string().assign(data.begin() + beg - index, data.begin() + end - index)});
        _accepted_len += end - beg;
        
        return end;
    }

    auto it = upper_bound(_data_set.begin(), _data_set.end(), beg, 
        [](const uint64_t &t, const std::pair<uint64_t, std::string> &p) {
            return t < p.first;
        }
    );

    if (it == _data_set.end()) {    //data_set内没有左端点比beg大的
        
        if (it != _data_set.begin()) {  //如果不是第一个，则必有前驱
            it --;
            if (it->first + it->second.size() >= end)    //如果前一个string右端大于等于end（全部重叠）
                return end;
            if (it->first + it->second.size() > beg)    //如果前一个string右端大于beg（部分重叠）
                beg = it->first + it->second.size();
        }

        //插入
        _data_set.insert({beg, string().assign(data.begin() + beg - index, data.begin() + end - index)});
        _accepted_len += end - beg;
        
        return end;
    }
    else {
        if (it != _data_set.begin()) {  //如果不是第一个，则必有前驱
            it --;
            if (it->first + it->second.size() >= end)    //如果前一个string右端大于等于end（全部重叠）
                return end;
            if (it->first + it->second.size() > beg)    //如果前一个string右端大于beg（部分重叠）
                return it->first + it->second.size();
            it ++;
        }
    

        //std::cout << "here : " << it->first << ' ' << end << " \r\n";
        if (it->first < end)  //如果小于end
            end = it->first;

        //插入
    
        _data_set.insert({beg, string().assign(data.begin() + beg - index, data.begin() + end - index)});
        _accepted_len += end - beg;
    
        return end;

    }

     
}