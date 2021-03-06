#include "standard.h"
#include "log.h"
#include "util.h"
#include "ctype.h"

void * xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p)
        cerror("malloc");
    return p;
}

void * xcalloc(size_t size) {
    void *p = calloc(1, size);
    if (!p)
        cerror("calloc");
    return p;
}

void * xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p)
        cerror("realloc");
    return p;
}

char * xstrdup(char const *src) {
    char *p = strdup(src);
    if (!p)
        cerror("strdup");
    return p;
}

vector_t strsplit(char *str, char const *sep) {
    vector_t res = vector_init(sizeof(char *));
    char * token;

    for (token = strtok(str, sep);
         token;
         token = strtok(NULL, sep))
        vector_push(res, &token);

    return res;
}

char * strlower(char *str) {
    char *p;
    for (p = str; p && *p; ++p)
        if (*p > 0)
            *p = tolower((int) *p);
    return str;
}

char * strtrim(char *str) {
    char *p = str + strlen(str) - 1;
    while (p >= str && (*p == 10 || *p == 13))
        *p-- = 0;
    return str;
}

unsigned strhash(char const * key) {
    unsigned res = 0;
    char const *p = key;

    while (*p)
        res = res * 37 + *p++;

    return res;
}

void const * read_file(char const *filename) {
    FILE *f;
    int size;
    void *res;

    f = fopen(filename, "rt");
    if (!f)
        cerror("can't open %s", filename);

    if (fseek(f, 0, SEEK_END))
        cerror("fseek");

    size = (int) ftell(f);
    if (size == -1)
        cerror("ftell");

    rewind(f);
    res = xmalloc(size);

    fread(res, 1, size, f);
    if (ferror(f))
        cerror("fread");

    return res;
}

#ifdef _MSC_VER

#include <windows.h>

void const * map_file_read(char const *filename, int populate_data) {
    void const *res = 0;
    HANDLE file, mapping;

    file = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE)
        error("can't open file %s", filename);

    mapping = CreateFileMapping(file, 0, PAGE_READONLY, 0, 0, NULL);
    if (!mapping)
        error("can't create file mapping");

    res = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!res)
        error("can't map file");

    return res;
}

#else

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

void const * map_file_read(char const *filename, int populate_data) {
    void *addr;
    struct stat sb;

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        cerror("can't open %s", filename);

    if (fstat(fd, &sb) == -1)
        cerror("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ,
                MAP_PRIVATE | (populate_data ? MAP_POPULATE : 0), fd, 0);
    if (addr == MAP_FAILED)
        cerror("mmap");

    if (madvise(addr, sb.st_size, MADV_WILLNEED | MADV_RANDOM))
        cerror("madvise");

    return addr;
}

#endif
