#include "byte_stream.hh"
#include "comm.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : buf_({}), cap_(capacity), 
  nread_(0), nwrite_(0), ended_(false)
{
  dbg_mutex_init();
}

size_t ByteStream::write(const string &data) {
  size_t len = data.length();

  /* 有多少容量写多少数据 */
  if(remaining_capacity() < data.length()) {
    len = remaining_capacity();
  }

  for(size_t i=0; i<len; i++) {
    buf_.push_back(data[i]);
  }

  nwrite_ += len;
  return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const 
{
  if(len > buf_.size()) {
    dbg_printf(DBG_ERR, "[%s][%d]ERROR: len > buf_'s size.\n", __FUNCTION__, __LINE__);    
    return "";
  }

  return string(buf_.begin(), buf_.begin()+len);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{
  if(len > buf_.size()) {
    dbg_printf(DBG_ERR, "[%s][%d]ERROR: len > buf_'s size.\n", __FUNCTION__, __LINE__);    
    return;
  }

  for(size_t i=0; i<len; i++) {
    buf_.pop_front();
  }

  nread_ += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) 
{
  string result = peek_output(len);
  
  pop_output(len);
  return result;
}

void ByteStream::end_input() 
{
  ended_ = true;
}

bool ByteStream::input_ended() const 
{ 
  return ended_; 
}

size_t ByteStream::buffer_size() const 
{ 
  return buf_.size(); 
}

bool ByteStream::buffer_empty() const 
{ 
  return buf_.empty(); 
}

bool ByteStream::eof() const 
{ 
  return ended_ && buf_.empty(); 
}

size_t ByteStream::bytes_written() const 
{
  return nwrite_; 
}

size_t ByteStream::bytes_read() const 
{ 
  return nread_; 
}

size_t ByteStream::remaining_capacity() const 
{ 
  return cap_ - buf_.size();  
}
