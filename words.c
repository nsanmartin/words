#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <rax.h>

enum { BufSz = 256 };

typedef struct { 
    const char* cs;
    size_t sz;
} StrView;

StrView findNextSubStrOrLastIxSpace(const char* s, const char* end) {

    while (s < end && isspace(*s)) { ++s; }

    StrView res = (StrView) { .cs=s, .sz=0 };
    if (!*s) { /* return empty string */ return res; }

    ++s;

    for (; s < end && !isspace(*s); ++s)
        ;
    res.sz = s-res.cs;

    return res;
}

int print_strview(StrView s) { return fwrite(s.cs, 1, s.sz, stdout); }

void* One = (void*) 1;

int readStdin(rax* rax) {
    char buf[BufSz];
    size_t read;
    while ((read = fread(buf, 1, BufSz, stdin))) {
        if (ferror(stdin)) { perror("error reading stdin"); return -1;}

        const char* end = buf + read;
        StrView word = (StrView){.cs=buf, .sz=0};
        do {
            StrView next = findNextSubStrOrLastIxSpace(word.cs+word.sz, end);
            if (next.cs + next.sz >= end) {
                memmove(buf, next.cs, next.sz);
                read = fread(buf+next.sz, 1, BufSz-next.sz, stdin);
                if (ferror(stdin)) { perror("error reading stdin"); return -1;}

                end = buf+next.sz + read;
                word = (StrView){.cs=buf, .sz=0};
                if (read == 0) { break; }
                else {
                    next = findNextSubStrOrLastIxSpace(word.cs+word.sz, end);
                    if (next.cs + next.sz >= end) {
                        puts("word too large, not supported.");
                        return -1;
                    }
                }
            }
            word = next;

            void* count = raxFind(rax, (unsigned char*)word.cs, word.sz);
            if (count == raxNotFound) {
                raxInsert(rax, (unsigned char*)word.cs, word.sz, One, NULL);
            } else {
                count = (void*) ((size_t)count + 1);
                if ((size_t)count == SIZE_MAX) {
                    fprintf(stderr, "error: size_t overflow\n");
                    return -1;
                }
                raxInsert(rax, (unsigned char*)word.cs, word.sz, count, NULL);
            }
            if (errno == ENOMEM) {
                perror("error: not enought memory");
                return -1;
            }
        } while (word.sz > 0);
    }
    return 0;
}

int main(void) {
    rax *rax = raxNew();
    int err = readStdin(rax);

    if(!err) {
        raxIterator iter;
        raxStart(&iter, rax); // Note that 'rt' is the radix tree pointer.
        raxSeek(&iter,"^",0x0,0);
        while(raxNext(&iter)) {
            fwrite(iter.key, 1, iter.key_len, stdout); 
            fwrite(": ", 1, 2, stdout); 
            printf("%ld\n", (size_t)iter.data);
        }
    }
}
