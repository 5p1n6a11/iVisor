# iVisor

## Virtualization Technology

```
$ grep -E 'svm|vmx' /proc/cpuinfo
```

## build

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

BitVisor のコンパイル

https://qiita.com/shina/items/8579d1c17962b0842431
