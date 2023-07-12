#pragma once

/**
 * This class is borrowed from menu_manager.cpp in the foobar2000 SDK.
 *
 * The following licence applies:
 *
 * foobar2000 1.4 SDK
 * Copyright (c) 2001-2018, Peter Pawlowski
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following
 * disclaimer.
 *
 * Neither the name of the author nor the names of its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
class MnemonicManager {
    pfc::string8_fast_aggressive used;
    bool is_used(unsigned c)
    {
        char temp[8];
        temp[pfc::utf8_encode_char(uCharLower(c), temp)] = 0;
        return !!strstr(used, temp);
    }

    static bool is_alphanumeric(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    }

    void insert(const char* src, unsigned idx, pfc::string_base& out)
    {
        out.reset();
        out.add_string(src, idx);
        out.add_string("&");
        out.add_string(src + idx);
        used.add_char(uCharLower(src[idx]));
    }

public:
    bool check_string(const char* src)
    { // check for existing mnemonics
        const char* ptr = src;
        while ((ptr = strchr(ptr, '&'))) {
            if (ptr[1] == '&')
                ptr += 2;
            else {
                unsigned c = 0;
                if (pfc::utf8_decode_char(ptr + 1, c) > 0) {
                    if (!is_used(c))
                        used.add_char(uCharLower(c));
                }
                return true;
            }
        }
        return false;
    }
    bool process_string(const char* src, pfc::string_base& out) // returns if changed
    {
        if (check_string(src)) {
            out = src;
            return false;
        }
        unsigned idx = 0;
        while (src[idx] == ' ')
            idx++;
        while (src[idx]) {
            if (is_alphanumeric(src[idx]) && !is_used(src[idx])) {
                insert(src, idx, out);
                return true;
            }

            while (src[idx] && src[idx] != ' ' && src[idx] != '\t')
                idx++;
            if (src[idx] == '\t')
                break;
            while (src[idx] == ' ')
                idx++;
        }

        // no success picking first letter of one of words
        idx = 0;
        while (src[idx]) {
            if (src[idx] == '\t')
                break;
            if (is_alphanumeric(src[idx]) && !is_used(src[idx])) {
                insert(src, idx, out);
                return true;
            }
            idx++;
        }

        // giving up
        out = src;
        return false;
    }
};
