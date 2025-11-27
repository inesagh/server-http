#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "files.h"
#include <stdio.h>
#include <stdlib.h>

ssize_t read_line(int fd, char* buffer, size_t maxLength) {
    size_t i = 0;

    while (i < maxLength - 1) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);

        if (n == 0) {           
            if (i == 0) return 0;
            break;
        }

        if (n < 0) {
            return -1;
        }

        buffer[i++] = ch;
        if (ch == '\n') break;  
    }

    buffer[i] = '\0';
    return i;
}

int read_from(int fd, void* buffer, size_t length) {
    size_t total = 0;
    char *p = buffer;

    while (total < length) {
        ssize_t n = recv(fd, p + total, length - total, 0);
        if (n <= 0) return -1; 
        total += n;
    }

    return 0;
}

int write_into(int fd, const void* buffer, size_t length) {
    size_t total = 0;
    const char *p = buffer;

    while (total < length) {
        ssize_t n = send(fd, p + total, length - total, 0);
        if (n <= 0) return -1;
        total += n;
    }

    return 0;
}

int read_file_to_buffer(const char* path, char** outBuf, size_t* outSize) {
    *outBuf = NULL;
    *outSize = 0;

    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }

    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return -1;
    }
    rewind(f);

    char *buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }

    size_t readBytes = fread(buf, 1, (size_t)sz, f);
    fclose(f);

    if (readBytes != (size_t)sz) {
        free(buf);
        return -1;
    }

    buf[sz] = '\0';
    *outBuf = buf;
    *outSize = (size_t)sz;
    return 0;
}

