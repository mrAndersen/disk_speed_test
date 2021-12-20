#include <iostream>
#include <ctime>
#include <cmath>
#include <sys/uio.h>

#define FILENAME "test.bin"

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef int i32;


u64 get_nano() {
    timespec result = {};
    clock_gettime(CLOCK_MONOTONIC_RAW, &result);
    return result.tv_sec * 1000000000 + result.tv_nsec;
}

u64 get_milli() {
    u64 nano = get_nano();
    return (u64) nano / 1000000;
}

void sleep(u64 millis) {
    double seconds = floor((double) millis / 1000.0f);
    double reminder = (double) millis - seconds * 1000.0f;

    timespec spec = {};
    spec.tv_sec = (long) seconds;
    spec.tv_nsec = (long) (reminder * 1000000);
    nanosleep(&spec, nullptr);
}

void truncate(iovec *vecs, i32 vec_size) {
    FILE *ptr = fopen(FILENAME, "w+");
    fclose(ptr);

    for (int i = 0; i < vec_size; i++) {
        free(vecs[i].iov_base);
    }

    free(vecs);
}

double test_speed(u32 chunks, u32 chunkSize) {
    FILE *pFile = fopen(FILENAME, "w+");
    int fd = fileno(pFile);

    auto *vecs = (iovec *) malloc(sizeof(iovec) * chunks);

    for (i32 i = 0; i < chunks; i++) {
        vecs[i].iov_base = malloc(chunkSize * chunks);
        vecs[i].iov_len = chunkSize;
    }

    u64 start = get_nano();
    writev(fd, &vecs[0], chunks);
    u64 end = get_nano() - start;

    double total_mb = (double) (chunkSize * chunks) / 1024.0f / 1024.0f;
    double total_sec = (double) end / 1000000000.0f;

    fflush(nullptr);

    printf("%d chunks | %.2f Mb size each %.2f Mb/s\n", chunks, (double) chunkSize / 1024.0f / 1024.0f, (total_mb / total_sec));
    truncate(vecs, chunks);

    return total_sec;
}

int main() {
    u32 size = 1024 * 1024;
    u32 chunks = 1;

    u64 total_bytes = 0;
    double total_write = 0;

    bool running = true;

    while (running) {
        total_write += test_speed(chunks, size);
        total_bytes += (chunks * size);

        chunks++;
        size *= 2;

        if (chunks > 10) {
            running = false;
        }
    }

    running = true;
    chunks = 20;
    size = 1024 * 1024 * 25;

    while (running) {
        total_write += test_speed(chunks, size);
        total_bytes += (chunks * size);

        chunks += 20;

        if (chunks > 200) {
            running = false;
        }
    }

    printf("%.2f Mb/s average\n", ((double) total_bytes / 1024.0f / 1024.0f / total_write));
}
