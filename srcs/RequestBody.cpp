#include "RequestBody.hpp"
#include <algorithm>
#include <unistd.h>
#include <iostream>

RequestBody::RequestBody(void)
{
    char filename[] = "body.XXXXXX";
    close(mkstemp(filename));
    _filename = filename;
    reset();
}

RequestBody::~RequestBody(void)
{
    if (_file.is_open())
        _file.close();
    if (_filename.size() > 0)
        unlink(_filename.c_str());
}

void RequestBody::reset(void)
{
    _error = 0;
    _consume_crlf = 0;
    _temp_chunk_size = 0;
    _expected_length = 0;
    _saved_length = 0;
    _need_end_chunk = false;
    _total_crlf = false;
    if (_file.is_open())
        _file.close();
    _file.open(_filename.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
}

void RequestBody::init_chunk(void)
{
    reset();
    _need_end_chunk = true;
}

void RequestBody::init_nonchunk(size_t content_length)
{
    reset();
    _expected_length = content_length;
}

bool RequestBody::add_chunk(const char *buffer, std::size_t size)
{
    const char *end = buffer + size;
    while (buffer < end)
    {
        if (_consume_crlf)
        {
            ++_total_crlf;
            --_consume_crlf;
            ++buffer;
        }
        else if (_expected_length > _saved_length)
        {
            size_t write_size
                = std::min(_expected_length - _saved_length, static_cast<size_t>(end - buffer));
            _file.write(buffer, write_size);
            // check error
            buffer += write_size;
            _saved_length += write_size;
            if (_need_end_chunk && _expected_length == _saved_length)
                _consume_crlf = 2;
        }
        else
        {
            char ch = *buffer++;
            if ('0' <= ch && ch <= '9')
                _temp_chunk_size = _temp_chunk_size * 16 + (ch - '0');
            else if ('a' <= ch && ch <= 'f')
                _temp_chunk_size = _temp_chunk_size * 16 + (ch - 'a' + 10);
            else if ('A' <= ch && ch <= 'F')
                _temp_chunk_size = _temp_chunk_size * 16 + (ch - 'A' + 10);
            else if (ch == '\r')
            {
                ++_total_crlf;
                _expected_length += _temp_chunk_size;
                if (_need_end_chunk && _temp_chunk_size == 0)
                {
                    _need_end_chunk = false;
                    _consume_crlf = 3;
                }
                else
                    _consume_crlf = 1;
                _temp_chunk_size = 0;
                continue;
            }
            else
            {
                ++_error;
                // std::cerr << "unknown char " << ch << (int)ch << std::endl;
                // throw std::runtime_error("invalid chunk size");
                break;
            }
        }
    }
    return _error;
}

bool RequestBody::failed() const
{
    return (_error);
}

bool RequestBody::completed() const
{
    return (
        _expected_length == _saved_length
        && _consume_crlf == 0
        && _need_end_chunk == false);
}

std::fstream &RequestBody::get_stream()
{
    _file.seekg(0);
    return (_file);
}

std::size_t RequestBody::size()
{
    return (_file.tellg());
}

// int main()
// {
//     RequestBody r;

//     r.init_chunk();
//     r.add_chunk("5\r\n", 3);
//     std::cout << r.failed() << r.completed() << r.size() << std::endl;
//     r.add_chunk("12345\r\n", 7);
//     std::cout << r.failed() << r.completed() << r.size() << std::endl;
//     r.add_chunk("0\r\n", 3);
//     r.add_chunk("\r\n", 2);
//     std::cout << r.failed() << r.completed() << r.size() << std::endl;

//     r.init_chunk();
//     r.add_chunk("0", 1);
//     r.add_chunk("\r", 1);
//     r.add_chunk("\n", 1);
//     r.add_chunk("\r", 1);
//     r.add_chunk("\n", 1);
//     std::cout << r.failed() << r.completed() << r.size() << std::endl;

//     r.init_nonchunk(0);
//     std::cout << r.failed() << r.completed() << r.size() << std::endl;

//     r.add_chunk("\n", 1);
//     std::cout << r.failed() << r.completed() << r.size() << std::endl;
// }
