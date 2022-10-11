#define static_length(array) (sizeof(array) / sizeof((array)[0]))

#define sfor(static_iterable) auto it = (static_iterable); for(u32 i = 0; i < static_length(static_iterable); i++, it = (static_iterable) + i)
#define sfor_count(iterable, count) auto it = (iterable); for(u32 i = 0; i < count; i++, it = (iterable) + i)
#define lfor(linked_list) for(auto it = (linked_list);  it; it = it->next)
#define count_for(count) for(u32 i = 0;  i < count; i++)
#define for_count(count) for(u32 i = 0;  i < count; i++)
