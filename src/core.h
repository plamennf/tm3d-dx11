#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef int64_t  b64;
typedef int32_t  b32;
typedef int16_t  b16;
typedef int8_t   b8;

typedef double   f64;
typedef float    f32;

// Copy-paste from https://gist.github.com/andrewrk/ffb272748448174e6cdb4958dae9f3d8
// Defer macro/thing.

#define CONCAT_INTERNAL(x,y) x##y
#define CONCAT(x,y) CONCAT_INTERNAL(x,y)

template<typename T>
struct ExitScope {
    T lambda;
    ExitScope(T lambda):lambda(lambda){}
    ~ExitScope(){lambda();}
    ExitScope(const ExitScope&);
  private:
    ExitScope& operator =(const ExitScope&);
};
 
class ExitScopeHelp {
  public:
    template<typename T>
        ExitScope<T> operator+(T t){ return t;}
};
 
#define defer const auto& CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()

#define ArrayCount(array) (sizeof(array)/sizeof((array)[0]))

const float PI = 3.14159265359f;

inline int get_string_length(char *a) {
    if (!a) return 0;
    
    int length = 0;
    while (*a) {
        ++length;
        ++a;
    }
    return length;
}

inline bool strings_match(char *a, char *b) {
    if (!a || !b) return false;
    
    return strcmp(a, b) == 0;
}

inline char *copy_string(char *str) {
    int length = get_string_length(str);
    char *result = new char[length + 1];
    memcpy(result, str, (length + 1) * sizeof(char));
    return result;
}

inline void Clamp(float *value, float min, float max) {
    if (!value) return;
    
    if (*value < min) *value = min;
    else if (*value > max) *value = max;
}

inline void Clamp(int *value, int min, int max) {
    if (!value) return;

    if (*value < min) *value = min;
    else if (*value > max) *value = max;
}

inline char *find_character_from_right(char *s, char c) {
    return strrchr(s, c);
}

inline char *find_character_from_left(char *s, char c) {
    return strchr(s, c);
}

inline char *consume_next_line(char **text_ptr) {
    char *t = *text_ptr;
    if (!*t) return nullptr;

    char *s = t;

    while (*t && (*t != '\n') && (*t != '\r')) t++;

    char *end = t;
    if (*t) {
        end++;

        if (*t == '\r') {
            if (*end == '\n') ++end;
        }

        *t = '\0';
    }
    
    *text_ptr = end;
    
    return s;
}

inline char *eat_spaces(char *at) {
    if (!at) return nullptr;
    
    char *result = at;
    while (*result == ' ') {
        result++;
    }
    return result;
}

inline char *eat_trailing_spaces(char *at) {
    if (!at) return nullptr;

    int orig_len = get_string_length(at);
    int len = 0;
    while (at[orig_len - len] == ' ') {
        len++;
    }

    char *result = at;
    result[orig_len - len] = 0;
    
    return result;
}

inline char *break_by_spaces(char **text_ptr) {
    char *t = *text_ptr;
    if (!*t) return nullptr;

    char *s = t;

    char *end = t;
    if (*t) {
        while (*end && *end != ' ') {
            end++;
            t++;
        }

        if (*end == ' ') {
            end++;
        }

        *t = '\0';
    }

    *text_ptr = end;

    return s;
}

inline bool is_end_of_line(char c) {
    return (c == '\n') || (c == '\r');
}

inline bool is_whitespace(char c) {
    return (c == '\t') || (c == '\v') || (c == ' ') || is_end_of_line(c);
}

inline bool starts_with(char *string, char *substring) {
    if (!string || !substring) return false;

    while (*substring && *string) {
        if (*substring != *string) {
            return false;
        }
        
        string++;
        substring++;
    }

    if (!*string && *substring) return false;
    if (*string && !*substring) return true;
    return false;
}

// Copy-paste from https://github.com/raysan5/raylib/blob/master/src/rtext.c
inline int get_codepoint(char *text, int *bytes_processed) {
    int code = 0x3f;
    int octet = (u8)(text[0]);
    *bytes_processed = 1;

    if (octet <= 0x7f) {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    } else if ((octet & 0xe0) == 0xc0) {
        // Two octets

        // [0]xC2-DF     [1]UTF8-tail(x80-BF)
        u8 octet1 = text[1];
        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytes_processed = 2; return code; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf)) {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *bytes_processed = 2;
        }
    } else if ((octet & 0xf0) == 0xe0) {
        u8 octet1 = text[1];
        u8 octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytes_processed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytes_processed = 3; return code; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) {
            *bytes_processed = 2;
            return code;
        }

        if ((octet >= 0xe0) && (0 <= 0xef)) {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *bytes_processed = 3;
        }
    } else if ((octet & 0xf8) == 0xf0) {
        // Four octets
        if (octet > 0xf4) return code;

        u8 octet1 = text[1];
        u8 octet2 = '\0';
        u8 octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytes_processed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytes_processed = 3; return code; } // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytes_processed = 4; return code; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) {
            *bytes_processed = 2; return code; // Unexpected sequence
        }

        if (octet >= 0xf0) {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *bytes_processed = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f; // Codepoints after U+10ffff are invalid

    return code;
}

// Copy-paste from https://github.com/dwilliamson/GDMagArchive/blob/master/jan04_novideo/blow/Lerp%201%20(January%202004)/mprintf.h
const int MPRINTF_INITIAL_GUESS = 256;

inline char *mprintf(char *fmt, ...) {
    char *res = nullptr;
    int size = MPRINTF_INITIAL_GUESS;

    while (1) {
        res = new char[size];
		if (!res) return NULL;

		va_list ap;
		va_start(ap, fmt);

		int len = vsnprintf(res, size, fmt, ap);
		va_end(ap);

		if ((len >= 0) && (size >= len + 1)) {
			size = len;
			break;
		}

		delete [] res;
		size *= 2;							
    }

    return res;
}

inline char *mprintf_valist(char *fmt, va_list ap_orig) {
    char *res = nullptr;
    int size = MPRINTF_INITIAL_GUESS;

    while (1) {
        res = new char[size];
		if (!res) return NULL;

		va_list ap;
        ap = ap_orig;

		int len = vsnprintf(res, size, fmt, ap);
		va_end(ap);

		if ((len >= 0) && (size >= len + 1)) {
			size = len;
			break;
		}

		delete [] res;
		size *= 2;							
    }

    return res;
}

inline char *mprintf(int size, char *fmt, ...) {
    assert(size > 0);

    char *res = nullptr;

    while (1) {
        res = new char[size];
		if (!res) return NULL;

		va_list ap;
		va_start(ap, fmt);

		int len = vsnprintf(res, size, fmt, ap);
		va_end(ap);

		if ((len >= 0) && (size >= len + 1)) {
			size = len;
			break;
		}

		delete [] res;
		size *= 2;							
    }

    return res;
}

struct Time_Info {
    f64 last_time = 0.0;
    f64 current_dt;
};

enum Program_Mode {
    PROGRAM_MODE_GAME,
};

struct Core {
    bool should_quit;
    char *operating_folder;
    Time_Info time_info;
    Program_Mode program_mode = PROGRAM_MODE_GAME;
};

extern Core core;

struct Entity_Manager;
Entity_Manager *get_entity_manager();

#endif
