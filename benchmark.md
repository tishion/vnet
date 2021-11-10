## copy file with scp through physical network

```bash
$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt  100%  512MB  47.9MB/s   00:10    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  50.7MB/s   00:10    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  51.6MB/s   00:09    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  49.1MB/s   00:10    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  52.6MB/s   00:09    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  52.4MB/s   00:09    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  51.9MB/s   00:09    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  49.7MB/s   00:10    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  52.0MB/s   00:09    

$ scp sheen@172.16.203.128:/home/sheen/sample.txt ~/pn-sample.txt
sample.txt 100%  512MB  50.0MB/s   00:10 
```

## copy file with scp through vnet-select

```bash
$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  14.5MB/s   00:35    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  14.3MB/s   00:35    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  13.6MB/s   00:37    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  14.7MB/s   00:34    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  15.8MB/s   00:32    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  14.9MB/s   00:34    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  13.9MB/s   00:36    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  14.6MB/s   00:35    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  14.0MB/s   00:36    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt 100%  512MB  15.1MB/s   00:33  
```

## copy file with scp through vnet-epoll

```bash
$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  15.9MB/s   00:32    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  15.5MB/s   00:32    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  16.5MB/s   00:31    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  16.0MB/s   00:32    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  16.2MB/s   00:31    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  18.3MB/s   00:28    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  17.5MB/s   00:29    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  16.2MB/s   00:31    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  15.9MB/s   00:32    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  15.9MB/s   00:32
```
