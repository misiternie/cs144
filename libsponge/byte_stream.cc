#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity) { 
    
}

size_t ByteStream::write(const string &data) {
    size_t n = min(data.size(), remaining_capacity());

    for (size_t i = 0; i < n; i ++)
        _q.emplace_back(data[i]);
    _total_writed_len += n;

    return n;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t n = min(len, buffer_size());
    return string().assign(_q.end()-n, _q.end());
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t n = min(len, buffer_size());
    for (size_t i = 1; i <= n; i ++)
        _q.pop_front(), _total_read_len ++;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t n = min(len, buffer_size());

    string ans(_q.begin(), _q.begin() + n);
    pop_output(n);

    return ans;
}

void ByteStream::end_input() { _eof = true;}

bool ByteStream::input_ended() const { return _eof; }

size_t ByteStream::buffer_size() const { return _capacity - remaining_capacity(); }

bool ByteStream::buffer_empty() const { return remaining_capacity() == _capacity; }

bool ByteStream::eof() const { return buffer_empty() && input_ended(); }

size_t ByteStream::bytes_written() const { return _total_writed_len; }

size_t ByteStream::bytes_read() const { return _total_read_len; }

size_t ByteStream::remaining_capacity() const { return _capacity - _total_writed_len + _total_read_len; }
