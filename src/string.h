struct str {
    u32 count;
    char *buffer;
};

#define str_for(string) char it = string.buffer[0]; for (u32 i = 0; i < string.count; i++, it = string.buffer[i])


internal u32 to_int(char c) {
    u32 result = c - '0';
    return result;
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

