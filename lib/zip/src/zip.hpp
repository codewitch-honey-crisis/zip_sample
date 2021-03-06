#ifndef HTCW_ZIP_HPP
#define HTCW_ZIP_HPP
#ifdef ARDUINO
#include <Arduino.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#endif
#include <bits.hpp>
#include <stream.hpp>

namespace zip {
    static_assert(bits::endianness()!=bits::endian_mode::none,"Please define HTCW_LITTLE_ENDIAN or HTCW_BIG_ENDIAN before including zip to indicate the byte order of the platform.");
    enum struct zip_result {
        success = 0,
        invalid_argument = 1,
        invalid_archive = 2,
        not_supported = 3,
        io_error = 4,
        invalid_state = 5,
        out_of_memory = 6
    };
    namespace helpers {
        // fast-way is faster to check than jpeg huffman, but slow way is slower
        static const int fast_bits = 9; // accelerate all cases in default tables
        static const int fast_mask = ((1 << fast_bits) - 1);

        static const uint8_t distance_extra[] PROGMEM = 
        {
            0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
            10, 10, 11, 11, 12, 12, 13, 13
        };

        static const uint16_t length_base[] PROGMEM =
        {
            3, 4, 5, 6, 7, 8, 9, 10, 11, 13,
            15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
            67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
        };

        static const uint8_t length_extra[] PROGMEM =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4,
            4, 4, 4, 5, 5, 5, 5, 0, 0, 0
        };

        static const uint16_t distance_base[] PROGMEM =
        {
            1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
            257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193,
            12289, 16385, 24577, 0, 0
        };

        static const uint8_t length_dezigzag[] PROGMEM = 
        {
            16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2,
            14,
            1, 15
        };

        static const uint8_t default_length[] PROGMEM = {
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
            0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x07, 0x07, 0x07, 0x07,
            0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
            0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
            0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
        };
        static const uint8_t default_distance[] PROGMEM = {
            0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
            0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
            0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
            0x05, 0x05
        };
        static int bit_reverse16(int n) {
            n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
            n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
            n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
            n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
            return n;
        }

        static int bit_reverse(int v, int bits) {
            // to bit reverse n bits, reverse 16 and shift
            // e.g. 11 bits, bit reverse and shift away 5
            return bit_reverse16(v) >> (16 - bits);
        }
        struct huffman
        {
            static const size_t fast_size = 1 << fast_bits;
            uint16_t fast[fast_size];
            uint16_t first_code[16];
            uint16_t first_symbol[16];
            int max_code[17];
            uint8_t size[288];
            uint16_t value[288];
            huffman() {
                memset(size,0,sizeof(size));
                memset(value,0,sizeof(value));
            }
            inline uint8_t read_size_list(const uint8_t* size_list,int i) {
#ifdef ARDUINO
                return pgm_read_byte(size_list+i);
#else
                return size_list[i];
#endif
            }
            zip_result initialize(const uint8_t* size_list,size_t num)
            {
                int i;
                int k = 0;
                int next_code[16];
                int sizes[17];
                memset(sizes, 0, sizeof(sizes));
                memset(fast, 0xFF, sizeof(fast));
   
                // DEFLATE spec for generating codes
                for (i = 0; i < num; ++i)
                {
                    ++sizes[read_size_list(size_list,i)];
                }
                sizes[0] = 0;
                int code = 0;
                for (i = 1; i < 16; ++i)
                {
                    next_code[i] = code;
                    first_code[i] = (uint16_t)code;
                    first_symbol[i] = (uint16_t)k;
                    code = (code + sizes[i]);
                    if (sizes[i] != 0)
                    {
                        if (code - 1 >= (1 << i))
                        {
                            // bad codelengths
                            return zip_result::invalid_archive;
                        }
                    }
                    max_code[i] = code << (16 - i); // preshift for inner loop
                    code <<= 1;
                    k += sizes[i];
                }
                max_code[16] = 0x10000; // sentinel
                for (i = 0; i < num; ++i)
                {
                    int s = read_size_list(size_list,i);
                    if (s != 0)
                    {
                        int c = next_code[s] - first_code[s] + first_symbol[s];
                        size[c] = (uint8_t)s;
                        value[c] = (uint16_t)i;
                        if (s <= fast_bits)
                        {
                            int j = bit_reverse(next_code[s], s);
                            while (j < (1 << fast_bits))
                            {
                                fast[j] = (uint16_t)c;
                                j += (1 << s);
                            }
                        }
                        ++next_code[s];
                    }
                }
                return zip_result::success;
            }
        };
    }
    static zip_result inflate(io::stream* in,io::stream* out,long long int in_size=-1,void*(*allocator)(size_t)=malloc,void(*deallocator)(void*)=free) {
        struct context final {
            io::stream* in;
            long long int in_pos;
            long long int in_size;
            io::stream* out;
            long long int out_pos;
            uint8_t out_buffer[32768];
            size_t out_buffer_front;
            size_t out_buffer_count;

            int bits;
            uint32_t code_buffer;
            helpers::huffman distance;
            helpers::huffman length;
            inline context() {

            }
            int read_in() {
                uint8_t b=0;
                if(0>in_size || in_pos<in_size) {
                    in_pos+=in->read(&b,1);
                }
                return b;
            }
            int pop_out_buffer() {
                if(0==out_buffer_count) {
                    return 0;
                }
                uint8_t result = out_buffer[out_buffer_front % 32768];
                out_buffer_front = (out_buffer_front + 1) % 32768;
                --out_buffer_count;
                return result;
            }
            void write_out(int value) {
                if(out_buffer_count==32768) {
                    uint8_t b=(uint8_t)pop_out_buffer();
                    out->write(&b,1);
                }
                out_buffer[(out_buffer_front + out_buffer_count++) % 32768] = (uint8_t)value;
                ++out_pos;
            }
            void fill() {
                do {
                    code_buffer |= (uint32_t)(read_in() << bits);
                    bits += 8;
                } while (bits <= 24);
            }
            unsigned int next(int n)
            {
                if (bits < n) fill();
                unsigned int k = (unsigned int)(code_buffer & ((1 << n) - 1));
                code_buffer >>= n;
                bits -= n;
                return k;
            }
            int decode(helpers::huffman* z)
            {
                int s;
                if (bits < 16) fill();
                int b = z->fast[code_buffer & helpers::fast_mask];
                
                if (b < 0xffff)
                {
                    s = z->size[b];
                    code_buffer >>= s;
                    bits -= s;
                    int r =z->value[b];
                    if(r==258) {
                        r=258;
                    }
                    return r;
                }

                // not resolved by fast table, so compute it the slow way
                // use jpeg approach, which requires MSbits at top
                int k = helpers::bit_reverse((int)code_buffer, 16);
                for (s = helpers::fast_bits + 1; ; ++s)
                    if (k < z->max_code[s])
                        break;
                if (s == 16) return -1; // invalid code!
                // code size is s, so:
                b = (k >> (16 - s)) - z->first_code[s] + z->first_symbol[s];
                code_buffer >>= s;
                bits -= s;
                int r = z->value[b];
                return r;
            }
            inline int length_extra(int i) {
#ifdef ARDUINO
                return pgm_read_byte(helpers::length_extra+i);
#else
                return helpers::length_extra[i];
#endif
            }
            inline int distance_extra(int i) {
#ifdef ARDUINO
                return pgm_read_byte(helpers::distance_extra+i);
#else
                return helpers::distance_extra[i];
#endif
            }
            inline int length_base(int i) {
#ifdef ARDUINO
                return pgm_read_word(helpers::length_base+i);
#else
                return helpers::length_base[i];
#endif
            }
            inline int distance_base(int i) {
#ifdef ARDUINO
                return pgm_read_word(helpers::distance_base+i);
#else
                return helpers::distance_base[i];
#endif
            }
            zip_result parse_huffman_block()
            {
                for (; ; )
                {
                    int z = decode(&length);
                    if (z < 256)
                    {
                        if (z < 0)
                        {
                            // error in huffman codes
                            return zip_result::invalid_archive;
                        }
                        write_out(z);
                    }
                    else
                    {
                        if (z == 256) return zip_result::success;
                        z -= 257;
                        int len = length_base(z);
                        if (length_extra(z) != 0) len += (int)next(length_extra(z));
                        z = decode(&distance);
                        if (z < 0) {
                            // bad huffman code
                            return zip_result::invalid_archive;
                        }
                        int dist = distance_base(z);
                        if (distance_extra(z) != 0) dist += (int)next(distance_extra(z));
                        for (int i = 0; i < len; i++, dist++)
                        {
                            int j = out_buffer_front+(out_buffer_count - dist+i);
                            if (j < 0) {
                                // bad distance
                                return zip_result::invalid_archive;
                            }
                            write_out(out_buffer[j% 32768]);
                        }
                    }
                }
                return zip_result::success;
            }
            inline int length_dezigzag(int i) {
#ifdef ARDUINO
                return pgm_read_byte(helpers::length_dezigzag+i);
#else
                return helpers::length_dezigzag[i];
#endif
            }
            zip_result recompute() {
                uint8_t len_codes[286 + 32 + 137]; //padding for maximum single op
                uint8_t code_length_sizes[19];
                memset(code_length_sizes,0,sizeof(code_length_sizes));
                unsigned int hlit = next(5) + 257;
                unsigned int hdist = next(5) + 1;
                unsigned int hclen = next(4) + 4;

                for (int i = 0; i < hclen; ++i)
                    code_length_sizes[length_dezigzag(i)] = (uint8_t)next(3);

                helpers::huffman code_length;
                zip_result r = code_length.initialize(code_length_sizes,19);
                if(zip_result::success!=r) {
                    return r;
                }
                int n = 0;
                while (n < hlit + hdist)
                {
                    int c = decode(&code_length);
                    if (c < 16)
                        len_codes[n++] = (uint8_t)c;
                    else if (c == 16)
                    {
                        c = (int)next(2) + 3;
                        for (int i = 0; i < c; i++) len_codes[n + i] = len_codes[n - 1];
                        n += c;
                    }
                    else if (c == 17)
                    {
                        c = (int)next(3) + 3;
                        for (int i = 0; i < c; i++) len_codes[n + i] = 0;
                        n += c;
                    }
                    else
                    {
                        c = (int)next(7) + 11;
                        for (int i = 0; i < c; i++) len_codes[n + i] = 0;
                        n += c;
                    }
                }
                if (n != hlit + hdist)
                {
                    // bad codelengths
                    return zip_result::invalid_archive;
                }
                r=length.initialize(len_codes,(int)hlit);
                if(zip_result::success!=r) {
                    return r;
                }
                return distance.initialize(len_codes+hlit, (int)hdist);
            }
            zip_result parse_uncompressed_block() {
                uint8_t header[4];
                if ((bits & 7) != 0)
                    next(bits & 7); // discard
                // drain the bit-packed data into header
                int k = 0;
                while (bits > 0)
                {
                    header[k++] = (uint8_t)(code_buffer & 255); // wtf this warns?
                    code_buffer >>= 8;
                    bits -= 8;
                }
                // now fill header the normal way
                while (k < 4)
                {
                    header[k++] = read_in();
                }
                int len = header[1] * 256 + header[0];
                int nlen = header[3] * 256 + header[2];
                if (nlen != (len ^ 0xffff)) {
                    // zlib corrupt
                    return zip_result::invalid_archive;
                }
                for(int i = 0;i<len;++i) {
                    uint8_t b;
                    if((-1<in_pos && in_size>=in_pos) || 1!=in->read(&b,1)) {
                        return zip_result::invalid_archive;
                    }
                    ++in_pos;
                    write_out(b);
                }
                return zip_result::success;
            }
        };
        if(nullptr==in || in->caps().read==0 || nullptr==out || out->caps().write==0) {
            return zip_result::invalid_argument;
        }
        zip_result r;
        context* ctx = (context*)allocator(sizeof(context));
        if(nullptr==ctx) {
            return zip_result::out_of_memory;
        }
        ctx->in = in;
        ctx->in_pos = 0;
        ctx->in_size = in_size;
        ctx->out = out;
        ctx->out_buffer_front = 0;
        ctx->out_buffer_count = 0;
        ctx->out_pos = 0;
        ctx->bits=0;
        ctx->code_buffer=0;
        int final;
        do
        {
            final = ctx->next(1) != 0;
            int type = (int)ctx->next(2);
            if (type == 0)
            {
                ctx->parse_uncompressed_block();
            }
            else if (type == 3)
            {
                deallocator(ctx);
                // invalid block type
                return zip_result::invalid_archive;
            }
            else
            {
                if (type == 1)
                {
                    // use fixed code lengths
                    r=ctx->length.initialize (helpers::default_length,288);
                    if(zip_result::success!=r) {
                        deallocator(ctx);
                        return r;
                    }
                    r=ctx->distance.initialize(helpers::default_distance,32);
                    if(zip_result::success!=r) {
                        deallocator(ctx);
                        return r;
                    }
                }
                else
                {
                    r=ctx->recompute();
                    if(zip_result::success!=r) {
                        deallocator(ctx);
                        return r;
                    }
                }
                r=ctx->parse_huffman_block();
                if(zip_result::success!=r) {
                    deallocator(ctx);
                    return r;
                }
            }
        } while (!final);
        while(ctx->out_buffer_count!=0) {
            uint8_t b = (uint8_t)ctx->pop_out_buffer();
            if(1!=ctx->out->write(b)) {
                deallocator(ctx);
                return zip_result::io_error;
            }
        }
        deallocator(ctx);
        return zip_result::success;
    }
    class archive;
    class archive_entry;

    class archive_entry final {
        friend class archive;
        long long int m_local_header_offset;
        uint16_t m_compression_method;
        size_t m_compressed_size;
        size_t m_uncompressed_size;
        io::stream* m_stream;
       
    public:
        archive_entry() : m_stream(nullptr) {

        }
        inline bool initialized() const {
            return nullptr!=m_stream;
        }
        size_t copy_path(char* buffer,size_t size) const {
            if(nullptr==buffer || nullptr==m_stream || size==0) return 0;
            m_stream->seek(m_local_header_offset+26,io::seek_origin::start);
            io::stream_reader_le rdr(m_stream);
            uint16_t name_len;
            if(!rdr.read(&name_len)) {
                return 0;
            }
            m_stream->seek(2,io::seek_origin::current);
            size_t s = size<(name_len+1)?size:(name_len+1);
            s=m_stream->read((uint8_t*)buffer,s-1);
            buffer[s]='\0';
            return s;
        }
        inline size_t uncompressed_size() const {
            return m_uncompressed_size;
        }
        inline size_t compressed_size() const {
            return m_compressed_size;
        }
        zip_result extract(io::stream* out_stream,void*(*allocator)(size_t)=malloc,void(*deallocator)(void*)=free) const {
            if(nullptr==m_stream) {
                return zip_result::invalid_state;
            }
            if(nullptr==out_stream) {
                return zip_result::invalid_argument;
            }
            m_stream->seek(m_local_header_offset+26,io::seek_origin::start);
            io::stream_reader_le rdr(m_stream);
            uint16_t name_len;
            if(!rdr.read(&name_len)) {
                return zip_result::invalid_archive;
            }
            uint16_t extra_len;
            if(!rdr.read(&extra_len)) {
                return zip_result::invalid_archive;
            }
            m_stream->seek(name_len+extra_len,io::seek_origin::current);
            if(0==m_compression_method) {
                size_t s = m_uncompressed_size;
                uint8_t buf[512];
                while(0<s) {
                    size_t r = m_stream->read(buf,512);
                    if(r!=out_stream->write(buf,r)) {
                        return zip_result::io_error;
                    }
                    s-=r;
                }
                return zip_result::success;
            }
            return inflate(m_stream,out_stream,m_compressed_size,allocator,deallocator);
        }
    };
    class archive final {
        size_t m_entries_size;
        long long int m_offset;
        io::stream* m_stream;
    
        zip_result init(io::stream *stream) {
            if(nullptr!=m_stream) {
                return zip_result::invalid_state;
            }
            if(nullptr==stream || stream->caps().read==0 || stream->caps().seek==0) {
                return zip_result::invalid_argument;
            }
            // find the end of central directory record
            uint32_t signature;
            size_t offset;

            for (offset = 22;; ++offset) {
                if(offset>UINT16_MAX) {
                    return zip_result::invalid_archive;
                }
                stream->seek(-offset,io::seek_origin::end);
                if(sizeof(signature)!=stream->read((uint8_t*)&signature,sizeof(signature))) {
                    return zip_result::io_error;
                }
                if (signature == bits::from_le(0x06054B50))
                    break;
            }
            // read end of central directory record
            stream->seek(-offset, io::seek_origin::end);
            io::stream_reader_le rdr(stream);
            if(!rdr.read(&signature)) {
                return zip_result::io_error;
            }
            if (signature != 0x06054B50) {
                return zip_result::not_supported;
            }
            uint16_t disk_number;
            rdr.read(&disk_number);
            uint16_t cdr_disk_number;
            rdr.read(&cdr_disk_number);
            uint16_t disk_num_entries;
            rdr.read(&disk_num_entries);
            uint16_t num_entries;
            rdr.read(&num_entries);
            uint32_t cdr_size;
            rdr.read(&cdr_size);
            uint32_t cdr_offset;
            rdr.read(&cdr_offset);
            stream->seek(2, io::seek_origin::current);
            
            if (!(disk_number == 0 &&
                cdr_disk_number == 0 &&
                disk_num_entries == num_entries))
            {
                return zip_result::not_supported;
            }

            // check for zip64
            int is64 = num_entries == UINT16_MAX || cdr_offset == UINT32_MAX || cdr_size == UINT32_MAX;

           
            if (is64) {
#if HTCW_MAX_WORD >= 64
                    // zip64 end of central directory locator
                    stream->seek(-offset - 20, io::seek_origin::end);
                    rdr.read(&signature);
                    uint32_t eocdr_disk;
                    rdr.read(&eocdr_disk);
                    uint64_t eocdr_offset;
                    rdr.read(&eocdr_offset);
                    uint32_t num_disks;
                    rdr.read(&num_disks);

                    if (!(signature == 0x07064B50 &&
                        eocdr_disk == 0 &&
                        num_disks == 1))
                    {
                        return zip_result::not_supported;
                    }
                    // zip64 end of central directory record
                    stream->seek((long long int)eocdr_offset,io::seek_origin::start);
                    rdr.read(&signature);
                    uint64_t eocdr_size64;
                    rdr.read(&eocdr_size64);
                    uint16_t version64;
                    rdr.read(&version64);
                    uint16_t version_needed64;
                    rdr.read(&version_needed64);
                    uint32_t disk_number64;
                    rdr.read(&disk_number64);
                    uint32_t cdr_disk_number64;
                    rdr.read(&cdr_disk_number64);
                    uint64_t disk_num_entries64;
                    rdr.read(&disk_num_entries64);
                    uint64_t u64;
                    rdr.read(&u64);
                    m_entries_size = (size_t)u64;
                    uint64_t cdr_size64;
                    rdr.read(&cdr_size64);
                    rdr.read(&u64);
                    m_offset = (long long int)u64;
                    if (!(signature == 0x06064B50 &&
                        disk_number64 == 0 &&
                        cdr_disk_number64 == 0 &&
                        disk_num_entries64 == m_entries_size))
                    {
                        return zip_result::not_supported;
                    }
#else
                return zip_result::not_supported;
#endif
            } else {
                m_offset = cdr_offset;
                m_entries_size = num_entries;
            }
            m_stream = stream;
            return zip_result::success;   
        }
public:
        archive() : m_stream(nullptr) {

        }
        archive(io::stream* stream) {
            init(stream);
        }
        static zip_result open(io::stream* stream,archive* out_archive) {
            if(nullptr==out_archive || nullptr==stream) {
                return zip_result::invalid_argument;
            }
            return out_archive->init(stream);
        }
        inline bool initialized() const {
            return nullptr!=m_stream;
        }
        inline size_t entries_size() const {
            return m_entries_size;
        }
        zip_result entry(size_t index,archive_entry* out_entry) const {
            if(nullptr==out_entry) {
                return zip_result::invalid_argument;
            }
            if(nullptr==m_stream) {
                return zip_result::invalid_state;
            }
            if(index>=m_entries_size) {
                return zip_result::invalid_argument;
            }

            m_stream->seek(m_offset,io::seek_origin::start);
            uint32_t local_header_offset;
            io::stream_reader_le rdr(m_stream);
            for(size_t i = 0;i<=index;++i) {
                m_stream->seek(28, io::seek_origin::current);
                uint16_t file_name_length;
                if(!rdr.read(&file_name_length)) {
                    return zip_result::invalid_archive;
                }
                uint16_t extra_field_length;
                if(!rdr.read(&extra_field_length)) {
                    return zip_result::invalid_archive;
                }
                uint16_t file_comment_length;
                if(!rdr.read(&file_comment_length)) {
                    return zip_result::invalid_archive;
                }
                m_stream->seek(8, io::seek_origin::current);
                if(!rdr.read(&local_header_offset)) {
                    return zip_result::invalid_archive;
                }
                long long int adv = file_name_length + extra_field_length + file_comment_length;
                m_stream->seek(adv, io::seek_origin::current);
            }
            m_stream->seek(local_header_offset+8,io::seek_origin::start);
            
            if(!rdr.read(&out_entry->m_compression_method)) {
                return zip_result::invalid_archive;
            }
            m_stream->seek(8,io::seek_origin::current);
            uint32_t sz;
            if(!rdr.read(&sz)) {
                return zip_result::invalid_archive;
            }
            out_entry->m_compressed_size=(size_t)sz;
            if(!rdr.read(&sz)) {
                return zip_result::invalid_archive;
            }
            
            out_entry->m_local_header_offset =(long long int)local_header_offset;
            out_entry->m_uncompressed_size=(size_t)sz;
            out_entry->m_stream = m_stream; 
            return zip_result::success;
        }

    };
}
#endif