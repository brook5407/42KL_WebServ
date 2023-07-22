#ifndef REQUESTBODY_HPP
# define REQUESTBODY_HPP

# include <fstream>

class RequestBody
{
    public:
        RequestBody(void);
        ~RequestBody(void);
        void            reset(void);
        void            init_chunk(void);
        void            init_nonchunk(size_t content_length);
        bool            add_chunk(const char *buffer, std::size_t size);
        bool            failed() const;
        bool            completed() const;
        std::fstream    &get_stream();
        std::size_t     size();
        const std::string &get_filename(void) const
        {return _filename;};

    private:
        std::string     _filename;
        std::fstream    _file;
        bool            _need_end_chunk;
        int             _error;
        size_t          _expected_length;
        size_t          _saved_length;
        size_t          _consume_crlf;
        size_t          _total_crlf;
        size_t          _temp_chunk_size;
};

#endif
