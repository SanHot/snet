//
//  Buffer.h
//  snet
//
//  Created by San on 16/3/4.
//  Copyright © 2016年 ___SAN___. All rights reserved.
//

#ifndef Buffer_h
#define Buffer_h

#include "stdafx.h"
#include "Log.h"

class Buffer
{
public:
    Buffer() {
        m_buffer = NULL;
        m_alloc_size = 0;
        m_write_offset = 0;
    }
    ~Buffer() {
        m_alloc_size = 0;
        m_write_offset = 0;
        if (m_buffer) {
            free(m_buffer);
            m_buffer = NULL;
//            printf("free buffer\n");
        }
    }
    char* buffer() {return m_buffer;}
    uint32_t size() {return m_alloc_size;}
    uint32_t offset() {return m_write_offset;}
    void incWriteOffset(uint32_t len) {m_write_offset += len;}
    
    void extend(uint32_t len) {
        m_alloc_size = m_write_offset + len;
        m_alloc_size += m_alloc_size >> 2;
        char* new_buf = (char*)realloc(m_buffer, m_alloc_size);
        if(new_buf != NULL) {
            m_buffer = new_buf;
//            printf("alloc buffer\n");
        }
        else {
            LOG_STDERR("Buffer: realloc error");
        }
    }
    uint32_t write(void* buf, uint32_t len) {
        if (m_write_offset + len >= m_alloc_size)
            extend(len);
        if (buf)
            memcpy(m_buffer + m_write_offset, buf, len);
        m_write_offset += len;
        return len;
    }
    uint32_t read(void* buf, uint32_t len) {
        if (len > m_write_offset)
            len = m_write_offset;
        if (buf)
            memcpy(buf, m_buffer, len);
        m_write_offset -= len;
        memmove(m_buffer, m_buffer + len, m_write_offset);
        return len;
    }
    
    void clear() {
        m_write_offset = 0;
    }
private:
    DISALLOW_EVIL_CONSTRUCTORS(Buffer);
    char* m_buffer;
    uint32_t m_alloc_size;
    uint32_t m_write_offset;
};

#endif /* Buffer_h */
