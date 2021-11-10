## copy file with scp through physical network directly

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

## copy file with scp through vnet-epoll

```bash
$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  27.3MB/s   00:18    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  28.2MB/s   00:18    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  26.9MB/s   00:19    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  26.8MB/s   00:19    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  25.4MB/s   00:20    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  24.1MB/s   00:21    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  27.8MB/s   00:18    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  23.6MB/s   00:21    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  27.0MB/s   00:18    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-epoll-sample.txt
sample.txt 100%  512MB  27.5MB/s   00:18    
```

## copy file with scp through vnet-select

```bash
$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  19.0MB/s   00:26    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  23.4MB/s   00:21    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  24.4MB/s   00:20    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  24.7MB/s   00:20    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  24.5MB/s   00:20    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  26.4MB/s   00:19    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  26.3MB/s   00:19    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  26.3MB/s   00:19    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  24.2MB/s   00:21    

$ scp sheen@10.0.0.11:/home/sheen/sample.txt ~/vn-select-sample.txt
sample.txt  512MB  26.6MB/s   00:19 
```

