export LC_ALL=C
SHELL:=/bin/bash

RK_SDK_BASE ?= /opt/jetkvm-native-buildkit
RK_APP_CROSS := $(RK_SDK_BASE)/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf

CC = $(RK_APP_CROSS)-gcc

CFLAGS = -Ideps/alsa-lib-1.2.14/include -Ideps/opus-1.5.2/include -Ideps/opus-1.5.2/celt
LDFLAGS = -Ldeps/opus-1.5.2/.libs -lopus -Ldeps/alsa-lib-1.2.14/src/.libs -lasound -O3 -g0 -static
BIN 			= jetkvm_audio

#Collect the files to compile
MAINSRC = $(wildcard ./*.c)
BUILD_DIR 		= ./build
BUILD_OBJ_DIR 	= $(BUILD_DIR)/obj
BUILD_BIN_DIR 	= $(BUILD_DIR)/bin

OBJEXT 			?= .o

AOBJS 			= $(ASRCS:.S=$(OBJEXT))
COBJS 			= $(CSRCS:.c=$(OBJEXT))

MAINOBJ 		= $(MAINSRC:.c=$(OBJEXT))

SRCS 			= $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS 			= $(AOBJS) $(COBJS) $(MAINOBJ)
TARGET 			= $(addprefix $(BUILD_OBJ_DIR)/, $(patsubst ./%, %, $(OBJS)))

all: default

$(BUILD_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"

default: $(TARGET)
	@mkdir -p $(dir $(BUILD_BIN_DIR)/)
	$(CC) -o $(BUILD_BIN_DIR)/$(BIN) $(TARGET) $(LDFLAGS)

clean:
	@echo "clean"
	@rm -rf build
