# Neumann (新架构)

## 概述

Neumann是一个开源图形化操作系统，名字是为了纪念计算机之父——冯·诺伊曼。使用C语言和NASM语法汇编编写（Beta6+,Beta5以下版本基于HariboteOS）

## 如何编译

### 一般方法

你需要准备一台运行Linux的电脑（以Ubuntu 20.04 LTS 64位为例）（Windows和Mac用户可以装虚拟机），如果你的系统有适用于Windows的Linux子系统（以下简称WSL），可以尝试这么做，不过作者不建议这样，因为作者没有用WSL测试过。

首先，要准备编译环境。你需要以下工具（apt软件包名称）：

gcc nasm make mtools qemu qemu-system-x86 

如果没有，执行以下命令

```bash
sudo apt install gcc nasm make mtools qemu qemu-system-x86 gcc-multilib
```

其中，gcc-multilib是修复以下报错的：

```
 fatal error: bits/libc-header-start.h: No such file or directory
```

接着，切换到你的代码目录

```bash
cd 源码目录
```

接着，输入以下命令便可以开始编译

```bash
make full
```

> [!NOTE]
>
> make时，你可以这么做：
>
> ```bash
> make full -j[CPU核心数]
> ```
>
> 这么做可以加快编译速度。
>
> 比如：
>
> ```bash
> make full -j8
> ```

如果提示没有报错，请看看目录下是否出现<kbd>boothd.img</kbd>这个文件。如果说出现了，那么恭喜你，你成功地编译了这个系统。

你可以输入以下命令来运行：

```bash
make run
```

> [!NOTE]
>
> 清理命令：
>
> ```bash
> make clean_full
> ```
>
> 一步到位命令：
>
> ```bash
> make fastbuild
> ```



### 生成其它格式映像文件

vhd格式：

```bash
make to_vhd
```

vmdk格式：

```bash
make to_vmdk
```

> [!IMPORTANT]
>
> 你必须编译<kbd>boothd.img</kbd>这个文件后，才能运行上述两个命令



## 源代码贡献者

wrhmade(GitHub用户名)——大部分架构

CandleOS-Yang(GiuHub用户名)——图层管理模块

FengHeting(GiuHub用户名)——APIC/HPET开发
