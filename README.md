# 简易版http服务器

## 功能

- [x] 0. 上传文件
- [ ] 1. 显示当前服务器上的文件列表
- [ ] 2. 下载文件
- [ ] 3. 并发
- [ ] 4. 持续连接、分块传输、openssl加密

编译:

```[bash]
cmake .
make
```

清理重新编译:

```[bash]
make clean
```

运行需要超级用户权限:

```[bash]
sudo ./server 
```
