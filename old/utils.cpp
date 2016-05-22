/*!
 * \file       utils.cpp
 * \brief      Implements the methods used to manipulate objects.
 * \author     Enrico Fraccaroli
 * \date       23 Agosto 2014
 * \copyright
 *  RadMud (C) Copyright 2014 by Enrico Fraccaroli.
 *  Permission to copy, use, modify, sell and distribute this software is granted
 *  provided this copyright notice appears in all copies. This software is provided
 *  "as is" without express or implied warranty, and with no claim as to its suitability
 *  for any purpose.
 */

#include "utils.h"

#include <iostream>
#include <cstdlib>
#include <zlib.h>

/// Check if the return code from Zlib is an error.
#define ZCHECK_ERROR(err, msg)\
if (err != Z_OK) {\
    LogError(#msg" error: "#err);\
    exit(1);\
}\

std::string GetCurrentTime() {
    time_t now = time(NULL);
    tm * ptm = localtime(&now);
    char buffer[32];
    // Format: H:M
    strftime(buffer, 32, "%H:%M", ptm);
    return buffer;
}

std::string GetDate() {
    time_t now = time(NULL);
    tm * ptm = localtime(&now);
    char buffer[32];

    // Format: %Y_%m_%d_%H-%M
    strftime(buffer, 32, "%Y_%m_%d", ptm);
    return buffer;
}

void LogMessage(std::string log) {
    // Output the message in the prompt.
    std::cout << "[" << GetCurrentTime() << "]" << log << std::endl;
}

void LogError(std::string log) {
    LogMessage("[ERROR]" + log);
}

void DeflateStream(std::vector<uint8_t> & uncompressed, std::vector<uint8_t> & compressed) {
    // ZLib compression stream.
    z_stream c_stream;
    // The error code returned from every call to zlib
    int errmsg;

    // Clear the input vector.
    compressed.clear();

    c_stream.zalloc = (alloc_func) 0;
    c_stream.zfree = (free_func) 0;
    c_stream.opaque = (voidpf) 0;

    errmsg = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    ZCHECK_ERROR(errmsg, "deflateInit");

    c_stream.next_in = &uncompressed[0];
    c_stream.avail_in = uncompressed.size();

    for (;;) {
        uint8_t c_buffer[10] = { };
        c_stream.next_out = &c_buffer[0];
        c_stream.avail_out = 10;

        errmsg = deflate(&c_stream, Z_FINISH);
        if (errmsg == Z_STREAM_END) {
            for (unsigned int i = 0; i < (10 - c_stream.avail_out); i++) {
                compressed.push_back(c_buffer[i]);
            }
            break;
        }
        ZCHECK_ERROR(errmsg, "deflate");
        for (unsigned int i = 0; i < (10 - c_stream.avail_out); i++) {
            compressed.push_back(c_buffer[i]);
        }
    }

    errmsg = deflateReset(&c_stream);
    ZCHECK_ERROR(errmsg, "deflateReset");
}

void InflateStream(std::vector<uint8_t> & compressed, std::vector<uint8_t> & uncompressed) {
    // ZLib decompression stream.
    z_stream d_stream;
    // The error code returned from every call to zlib
    int errmsg;

    // Clear the input vector.
    uncompressed.clear();

    d_stream.zalloc = Z_NULL;
    d_stream.zfree = Z_NULL;
    d_stream.opaque = Z_NULL;
    d_stream.avail_in = 0;
    d_stream.next_in = Z_NULL;

    errmsg = inflateInit(&d_stream);
    ZCHECK_ERROR(errmsg, "inflateInit");

    d_stream.avail_in = compressed.size();
    d_stream.next_in = &compressed[0];

    do {
        uint8_t d_buffer[10] = { };
        d_stream.next_out = &d_buffer[0];
        d_stream.avail_out = 10;

        errmsg = inflate(&d_stream, Z_NO_FLUSH);
        if (errmsg == Z_STREAM_END) {
            for (unsigned int i = 0; i < (10 - d_stream.avail_out); i++) {
                uncompressed.push_back(d_buffer[i]);
            }
            if (d_stream.avail_in == 0) {
                break;
            }
        }
        else {
            ZCHECK_ERROR(errmsg, "inflate");
        }

        for (unsigned int i = 0; i < (10 - d_stream.avail_out); i++) {
            uncompressed.push_back(d_buffer[i]);
        }
    }
    while (d_stream.avail_out == 0);

    errmsg = inflateReset(&d_stream);
    ZCHECK_ERROR(errmsg, "inflateEnd");
}
