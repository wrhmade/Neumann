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
sudo apt install gcc nasm make mtools qemu qemu-system-x86 
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

如果提示没有报错，请看看目录下是否出现一个叫做<kbd>bootfd.img</kbd>的文件。如果说出现了，那么恭喜你，你成功地编译了这个系统。

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

### 生成光盘映像

要想生成光盘映像，你可以在bash下输入：

```bash
bash ./mkiso.sh
```

该命令会生成一个叫<kbd>bootcd.iso</kbd>的iso映像文件，你可以自行刻录到CD上并从光盘启动。

## 源代码贡献者

wrhmade(GitHub用户名)——大部分架构

Dashboard(GiuHub用户名)——图层管理模块
