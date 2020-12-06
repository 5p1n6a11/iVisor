# iVisor

### Virtualization Technology

```
$ grep -E 'svm|vmx' /proc/cpuinfo
```

### build

```
$ make
$ make -C boot/loader
$ make -C boot/uefi-loader
$ ls bitvisor.elf
bitbisor.elf
$ ls boot/loader/bootloader
boot/loader/bootloader
$ ls boot/loader/bootloaderusb
boot/loader/bootloaderusb
$ ls boot/uefi-loader/loadvmm.efi
boot/uefi-loader/loadvmm.efi
```

### install

Ubuntu 20.10 on VMware Workstation 16 Player

```
$ sudo cp bitvisor.elf /boot/
$ sudo vim /etc/grub.d/99_bitvisor
$ sudo chmod +x /etc/grub.d/99_bitvisor
$ sudo update-grub2
```

### test

```
$ cd tools/dbgsh
$ make
$ ./dbgsh
> log
> exit
```

## Thin Hypervisor

ksm

https://github.com/asamy/ksm

hvpp

https://github.com/wbenny/hvpp

HyperPlatform

https://github.com/tandasat/HyperPlatform

SimpleVisor

https://github.com/ionescu007/SimpleVisor

ShadowBox

https://github.com/kkamagui/shadow-box-for-x86

Bareflank

https://github.com/Bareflank/hypervisor

## BitVisor

BitVisor入門 -- イントロダクション

https://qiita.com/mmi/items/0b4fc2ac77a9d8e6ef9d

BitVisor入門 -- Getting started

https://qiita.com/mmi/items/5734db4ba8133a7758aa

BitVisor のコンパイル

https://qiita.com/shina/items/8579d1c17962b0842431

BitVisor のインストール（ローカルディスク）

https://qiita.com/shina/items/97b0ede50a6c1e85eb0a
