int
eat_string(u8** scanner, cstring pattern) {
    int count = 0;
    u8* scan = *scanner;
    u8* pattern_scan = (u8*) pattern;
    while (*pattern_scan) {
        count++;
        if (*scan++ != *pattern_scan++) {
            return false;
        }
    }
    string result;
    result.data = (u8*) *scanner;
    result.count = count;
    
    *scanner = scan;
    return true;
}

string
eat_until(u8** scanner, u8 end) {
    string result;
    result.data = (u8*) *scanner;
    
    u32 count = 0;
    u8* scan = *scanner;
    while (*scan) {
        if (*scan++ == end) {
            *scanner = scan;
            result.count = count;
            break;
        }
        count++;
    }
    
    return result;
}

inline string
eat_line(u8** scanner) {
    return eat_until(scanner, '\n');
}

// TODO(Alexander): WIP
#if 0
f32
eat_f32(char** scanner) {
    u8* scan = (u8*) *scanner;
    u32 curr_index = 0;
    f64 value = 0.0;
    u8 c = 0;
    while (curr_index < token.suffix_start) {
        c = token.source[curr_index++];
        if (c == '_') continue;
        if (c == 'e' || c == 'E' || c == '.') break;
        f64 d = (f64) hex_digit_to_s32(c);
        value = value * 10.0 + d;
    }
    
    if (c == '.') {
        f64 numerator = 0.0;
        f64 denominator = 1.0;
        while (curr_index < string_count(token.source)) {
            c = token.source[curr_index++];
            if (c == '_') continue;
            if (c == 'e' || c == 'E') break;
            numerator = numerator * 10.0 + (f64) (c - '0');
            denominator = denominator * 10.0;
        }
        value += numerator/denominator;
    }
    
    if (c == 'e' || c == 'E') {
        pln("token.source = %\n", f_string(token.source));
        c = token.source[curr_index++];
        f64 exponent = 0.0;
        f64 sign = 1.0;
        if (c == '-') sign = -1.0;
        
        while (curr_index < string_count(token.source)) {
            c = token.source[curr_index++];
            if (c == '_') continue;
            exponent = exponent * 10.0 + (f64) (c - '0');
        }
        value = value*pow(10.0, sign*exponent);
    }
}
#endif

int
eat_integer(u8** scanner) {
    int result = 0;
    bool negative = false;
    u8* scan = *scanner;
    if (*scan == '-') {
        negative = true;
        scan++;
    }
    
    for (; *scan; scan++) {
        if (*scan < '0' || *scan > '9') {
            break;
        }
        int number = (int) (*scan - '0');
        result = result*10 + number;
    }
    
    *scanner = scan;
    if (negative) {
        result = -result;
    }
    
    return result;
}
