CUSTOM_IO_EXT_VERSION = 1.0
CUSTOM_IO_EXT_MODULES = custom_io_ext
CUSTOM_IO_EXT_SITE = $(TOPDIR)/package/$(CUSTOM_IO_EXT_MODULES)
CUSTOM_IO_EXT_SITE_METHOD = local
# 使用 BUILD_DIR 变量来获取构建目录
CUSTOM_IO_EXT_BUILD_DIR = $(BUILD_DIR)/$(CUSTOM_IO_EXT_MODULES)-$(CUSTOM_IO_EXT_VERSION)

ifeq ($(BR2_CUSTOM_IO_EXT_BUILTIN),y)
define CUSTOM_IO_EXT_BUILD_CMDS
    $(MAKE) -C $(LINUX_DIR) M=$(shell pwd) modules
endef
endif

ifeq ($(BR2_CUSTOM_IO_EXT_MODULE),y)
define CUSTOM_IO_EXT_INSTALL_TARGET_CMDS
    $(shell mkdir -p $(TARGET_DIR)/lib/modules/$(LINUX_VERSION)/extra/)
    $(INSTALL) -m 644 $(CUSTOM_IO_EXT_BUILD_DIR)/$(CUSTOM_IO_EXT_MODULES).ko $(TARGET_DIR)/lib/modules/$(LINUX_VERSION)/extra/
endef
endif

$(eval $(kernel-module))
$(eval $(generic-package))

