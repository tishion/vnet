#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <vector>

int forward_with_rw(std::vector<uint8_t>& buf, int in_fd, int out_fd);

int forward_with_splice(int in_fd, int out_fd, int pipe[2]);

#endif /*UTILS_H*/