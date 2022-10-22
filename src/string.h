struct str {
    u32 count;
    char *buffer;
};

void printf(str s, bool print_count=true) {
    // this is for the debug() macro
    if (print_count) {
        printf("(%d)%.*s", s.count, s.count, s.buffer);
    } else {
        printf("%.*s", s.count, s.buffer);
    }
}

#if !defined(static_length)
    #define static_length(array) (sizeof(array) / sizeof((array)[0]))
#endif

#define str_for(string) char it = string.buffer[0]; for (u32 i = 0; i < string.count; i++, it = string.buffer[i])
#define STATIC_STR(zero_terminated_string) {static_length(zero_terminated_string) - 1, zero_terminated_string}
#define STR(zero_terminated_string) {length(zero_terminated_string), zero_terminated_string}


internal u32 to_int(char c) {
    u32 result = c - '0';
    return result;
}


#define isdigit(c) (c >= '0' && c <= '9')

internal f64 atof(char *s) {
    // https://github.com/GaloisInc/minlibc/blob/master/atof.c
    // look for ascii to float transformation and use a good implementation, i dont know if this code is a good implementation
    // This function stolen from either Rolf Neugebauer or Andrew Tolmach. 
    // Probably Rolf.
    f64 a = 0.0;
    s32 e = 0;
    s32 c;

    while ((c = *s++) != '\0' && isdigit(c)) {
        a = a*10.0 + (c - '0');
    }

    if (c == '.') {
        while ((c = *s++) != '\0' && isdigit(c)) {
            a = a*10.0 + (c - '0');
            e = e-1;
        }
    }

    if (c == 'e' || c == 'E') {
        s32 sign = 1;
        s32 i = 0;
        c = *s++;

        if (c == '+') {
            c = *s++;
        } else if (c == '-') {
            c = *s++;
            sign = -1;
        }

        while (isdigit(c)) {
            i = i*10 + (c - '0');
            c = *s++;
        }

        e += (i * sign);
    }

    while (e > 0) {
        a *= 10.0;
        e--;
    }

    while (e < 0) {
        a *= 0.1;
        e++;
    }

    return a;
}



internal bool is_eol(char c) {
    return (c == '\n');
}

internal bool is_whitespace(char c) {
    if (c == ' ') return true;
    if (c == '\t') return true;
    if (c == '\r') return true;
    return false;
}

internal bool is_alpha(char c) {
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c >= 'a') && (c <= 'z')) return true;
    return false;
}

internal bool is_numeric(char c) {
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

internal bool is_alpha_numeric(char c) {
    return (is_alpha(c) || is_numeric(c));
}

internal u32 length(char *buffer) {
    u64 c0 = (u64)buffer;

    while (*buffer) {
        buffer++;
    }

    u32 result = (u32)((u64)buffer - c0);
    return result;
}

internal bool match(str origin, str pattern) {
    // assert(origin.count >= pattern.count, "pattern in string match must be smaller than the origin");
    u32 i = 0;

    while ((i < origin.count) && (i < pattern.count)) {
        if (origin.buffer[i] != pattern.buffer[i]) {
            return false;
        }

        i++;
    }

    return true;
}

internal bool equals(str a, str b) {
    if (a.count != b.count) return false;

    str_for(a) {
        if (it != b.buffer[i]) return false;
    }

    return true;
}

internal str create_str(char *buffer) {
    str result;
    result.buffer = buffer;
    result.count = length(buffer);
    return result;
};

internal u32 length(str string) {
    return string.count;
}

internal u32 length(str *string) {
    return string->count;
}

internal void offset(str *string, u32 i) {
    assert(i <= string->count, "cannot offset an string more than its size");
    string->buffer += i;
    string->count -= i;
}

internal str offset(str string, u32 i) {
    assert(i <= string.count, "cannot offset an string more than its size");
    str result = string;

    result.buffer += i;
    result.count -= i;

    return result;
}

internal str slice(str source, u32 c0, u32 cf) {
    // source = {18, "this is an example"}, c0 = 5, cf = 10
    // result = {5,       "is an"}
    assert(c0 <= cf, "starting index in string slice must be lower than the finish index");
    assert(c0 >= 0, "start index in string slice must be higher or equal to 0");
    assert(c0 <= source.count, "start index in string slice must be lower than the source length");
    assert(cf >= 0, "finish index in string slice must be higher or equal to 0");
    assert(cf <= source.count, "finish index in string slice must be lower than the source length");

    str result = source;

    result.buffer += c0;
    result.count = cf - c0;

    return result;
}

internal u32 find_n_from_position(str source, u32 start, char match, u32 n, bool backwards) {
    s32 d = 1 + ((s32)backwards * (-2));
    s32 finish = backwards ? 0: source.count;
    u32 result = finish;
    u32 count = 0;

    assert(start >= 0, "start index in find_first_from_position must be higher or equal to 0");
    assert(start < source.count, "start index in find_first_from_position must be lower than the source length");

    for (s32 i = (s32)start; i != finish; i += d) {
        char c = source.buffer[i];

        if (c == match) {
            count++;

            if (count == n) {
                result = (u32)i;
                break;
            }
        }
    }

    return result;
}

internal u32 find_first_from_position(str source, u32 start, char match, bool backwards) {
    return find_n_from_position(source, start, match, 1, backwards);
}

struct Split_iterator {
    str data;
    str pattern;
    str sub_str;
    bool valid;
};

internal Split_iterator next(Split_iterator iterator) {
    Split_iterator result = iterator;
    result.sub_str = result.data;
    u32 count = 0;

    while (result.data.count > 0) {
        if (match(result.data, result.pattern)) {
            result.data = offset(result.data, length(result.pattern));
            break;
        }

        result.data = offset(result.data, 1);
        count++;
    }

    result.sub_str.count = count;

    return result;
}

internal Split_iterator split(str original, str pattern) {
    Split_iterator result;

    result.data = original;
    result.pattern = pattern;

    result = next(result);

    return result;
}

internal bool valid(Split_iterator *it) {
    bool result = it->valid;
    it->valid = it->data.count > 0;
    return result;
}

// Hash Functions
// http://www.cse.yorku.ca/~oz/hash.html

internal u32 hash(str string) {
    // djb2
    u32 hash = 5381;

    str_for (string) {
        hash = ((hash << 5) + hash) + it; /* hash * 33 + it */
    }

    return hash;
}

internal u32 hash2(str string) {
    // sdbm
    u32 hash = 0;

    str_for (string) {
        hash = it + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

internal u32 hash3(str string) {
    // lose lose
    u32 hash = 0;

    str_for (string) {
        hash += it;
    }

    return hash;
}


struct Str_buffer {
    u32 size;
    u32 head;
    char *buffer;
};

internal Str_buffer new_str_buffer(Memory_pool *allocator, size_t size) {
    Str_buffer result;

    result.size = size;
    result.head = 0;
    // for stb_sprintf zero terminate the buffer
    result.buffer = push_array(allocator, char, result.size + 1);
    result.buffer[result.size] = 0;

    return result;
}

internal char *get_buffer(Str_buffer *str) {
    char *result = str->buffer + str->head;
    return result;
}

