#ifndef UTILS_H
#define UTILS_H

#include <cstdint>

int forward_with_readwrite(uint8_t* buf, int size, int in_fd, int out_fd);

int forward_with_splice(int in_fd, int out_fd, int pipe[2]);

#endif /*UTILS_H*/