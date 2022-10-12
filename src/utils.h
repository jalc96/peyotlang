#define static_length(array) (sizeof(array) / sizeof((array)[0]))

#define sfor(static_iterable) auto it = (static_iterable); for(u32 i = 0; i < static_length(static_iterable); i++, it = (static_iterable) + i)
#define sfor_count(iterable, count) auto it = (iterable); for(u32 i = 0; i < count; i++, it = (iterable) + i)
#define lfor(linked_list) for(auto it = (linked_list);  it; it = it->next)
#define count_for(count) for(u32 i = 0;  i < count; i++)
#define for_count(count) for(u32 i = 0;  i < count; i++)

#define KILOBYTE 1024LL
#define MEGABYTE 1048576LL
#define GIGABYTE 1073741824LL
#define TERABYTE 1099511627776LL

#define KILOBYTES(count) ((count) * (KILOBYTE))
#define MEGABYTES(count) ((count) * (MEGABYTE))
#define GIGABYTES(count) ((count) * (GIGABYTE))
#define TERABYTES(count) ((count) * (TERABYTE))

#define al_min(a, b) ((a) < (b) ? (a) : (b))
#define al_max(a, b) ((a) < (b) ? (b) : (a))
#define al_clamp(n, min_n, max_n) al_min(al_max(n, min_n), max_n)
