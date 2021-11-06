# About
Test the TUN device performance on Linux


# Benchmark

## On 192.168.1.11
### 1. start vn-agent from
```bash
./out/bin/vnet-agent 8888 10.0.0.11 192.168.1.22
```

### 2. make sure sshd is running

## On 192.168.1.22
### 1. start vn-agent from machine 192.168.1.11
```bash
./out/bin/vnet-agent 8888 10.0.0.11 192.168.1.20
```
### 2. copy file with scp through physical network 
```bash
scp user@192.168.1.11:/home/user/file ./file
```

### 3. copy file with scp through vnet 
```bash
scp user@10.0.0.11:/home/user/file ./file
```

## Test Result
```bash
# copy file with scp through physical network
user@ubuntu:~/shares/Source/vnet$ scp user@192.168.1.11:/home/user/big-file ./big-file-pn
big-file                                                                    100% 4096MB 100.3MB/s   00:40    

# copy file with scp through vnet (epoll)
user@ubuntu:~/shares/Source/vnet$ scp user@10.0.0.11:/home/user/big-file ./big-file
big-file                                                                    100% 4096MB   9.0MB/s   07:35    

# copy file with scp through vnet (select)
user@ubuntu:~/shares/Source/vnet$ scp user@10.0.0.11:/home/user/big-file ./big-file-vn-select
big-file                                                                     14%  608MB  10.5MB/s   05:30    

```

