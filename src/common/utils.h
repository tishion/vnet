#ifndef UTILS_H
#define UTILS_H

int forward_with_rw(int in_fd, int out_fd);

int forward_with_splice(int in_fd, int out_fd, int pipe[2]);

#endif /*UTILS_H*/