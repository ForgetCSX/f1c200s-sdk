# 设置要编译的模块
obj-m := custom_io_ext.o

# 默认目标：编译模块
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# 清理目标
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
