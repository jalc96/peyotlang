// ANSI ESCAPE CODES: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
internal void color_text(char *text, u8 r, u8 g, u8 b) {
    printf("\033[38;2;%d;%d;%dm%s\033[0m", r, g, b, text);
}

#define BOLD(text_print) printf("\033[1m"); text_print; printf("\033[22m")
#define ITALIC(text_print) printf("\033[3m"); text_print; printf("\033[23m")
#define UNDERLINE(text_print) printf("\033[4m"); text_print; printf("\033[24m")

#define RED(text) color_text(text, 255, 0, 0)
#define YELLOW(text) color_text(text, 255, 255, 0)
#define DEBUG(text) ITALIC(UNDERLINE(BOLD(color_text(text, 255, 185, 0))))

void printf(char *string) { printf("%s", string); }
void printf(f32 number)   { printf("%.2f", number); }
void printf(f64 number)   { printf("%.2f", number); }
void printf(u8 number)    { printf("%u", number); }
void printf(u16 number)   { printf("%u", number); }
void printf(u32 number)   { printf("%u", number); }
void printf(u64 number)   { printf("%llu", number); }
void printf(s8 number)    { printf("%d", number); }
void printf(s16 number)   { printf("%d", number); }
void printf(s32 number)   { printf("%d", number); }
void printf(s64 number)   { printf("%lld", number); }
void printf(bool value)   { printf("%s", value ? "true" : "false"); }

#define debug(var) {DEBUG(#var); putchar(':'); putchar(' '); printf(var); putchar('\n');}
