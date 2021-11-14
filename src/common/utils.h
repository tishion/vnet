#ifndef UTILS_H
#define UTILS_H

#include <cstdint>

int forward_with_readwrite(uint8_t* buf, int size, int in_fd, int out_fd);

// currently the linux kernel build-in tun driver doesn't support
// splice operation, so we cannot leverage the zero copy technology
// for better performance to redirect the data, refer to :
// https://github.com/torvalds/linux/blob/master/drivers/net/tun.c
// https://core.ac.uk/download/pdf/44404755.pdf
// https://linuxgazette.net/149/misc/melinte/
// if performace is critically required then we need to implement
// a custom tun device dirver.
int forward_with_splice(int pipe[2], int in_fd, int in_flags, int out_fd, int out_flags);

#endif /*UTILS_H*/