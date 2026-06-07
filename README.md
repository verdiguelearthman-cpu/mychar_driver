# mychar - Linux 内核字符设备驱动程序

## 项目概述

本项目实现了一个最基本的 Linux 内核字符设备驱动程序 `mychar`，属于操作系统原理课程的实验项目。该驱动遵循 Linux 内核模块开发规范，实现了模块的加载与卸载生命周期管理，支持用户空间与内核空间之间的数据交互。

### 功能特性

- 动态分配设备号（`alloc_chrdev_region`）
- 注册字符设备（`cdev_init` / `cdev_add`）
- 自动创建设备节点（`class_create` / `device_create`）
- 四个核心文件操作接口：`open`、`read`、`write`、`release`
- 通过 `copy_from_user` / `copy_to_user` 安全传输数据
- 内核日志调试输出（`printk`）

## 环境要求

- Linux 内核 6.x（项目基于 6.6 开发测试）
- GCC 编译器
- GNU Make
- Linux 内核头文件（`linux-headers`）

### 开发环境

本项目在 Windows 10/11 + WSL2 (Ubuntu) 环境下开发测试。

如使用自定义内核源码树，需在 `Makefile` 中设置 `KDIR` 路径。

## 项目结构

```
mychar_driver/
|-- mychar.c          # 内核字符设备驱动程序源码
|-- test_maychar.c    # 用户空间测试程序
|-- Makefile          # 内核模块编译文件
|-- test_mychar       # 编译好的测试程序二进制
|-- mychar.ko         # 编译好的内核模块
|-- README.md         # 本文件
|-- (编译中间产物)
```

## 编译方法

```
cd ~/mychar_driver
make
```

编译成功后生成 `mychar.ko` 内核模块文件。

## 使用说明

### 1. 加载模块

```
sudo insmod mychar.ko
```

加载后自动创建设备节点：

```
ls -l /dev/mychar
```

### 2. 编译测试程序

```
gcc -o test_mychar test_maychar.c
```

### 3. 运行测试

```
sudo ./test_mychar
```

### 4. 查看内核日志

```
dmesg | tail -20
```

### 5. 卸载模块

```
sudo rmmod mychar
```

## 驱动架构

用户空间测试程序 -> VFS层系统调用接口 -> 内核空间字符设备驱动

### 数据流程

1. 模块加载: insmod -> my_init() -> alloc_chrdev_region() -> cdev_init/cdev_add -> class_create/device_create -> /dev/mychar
2. 数据写入: write() -> VFS -> my_write() -> copy_from_user() -> 内核缓冲区
3. 数据读取: read() -> VFS -> my_read() -> copy_to_user() -> 用户空间
4. 模块卸载: rmmod -> my_exit() -> device_destroy/class_destroy -> cdev_del -> unregister_chrdev_region()

## 测试验证

测试程序验证流程：

1. 打开设备 /dev/mychar
2. 写入 46 字节字符串
3. 关闭并重新打开设备（重置读偏移）
4. 读取数据
5. 通过 strcmp 比对写入与读取内容
6. 输出验证结果

## 许可证

本项目基于 GPL 许可证发布。

## 作者

操作系统原理课程实验 - 计算机科学与技术学院
