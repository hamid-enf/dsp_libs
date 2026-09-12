# ============================================================================
# STM32H7 DSP Library — Makefile
#
# اهداف:
#   make            — ساخت کتابخانه به صورت بایگانی (libdsp.a)
#   make test       — ساخت و اجرای همه‌ی تست‌ها (روی Host)
#   make bench      — ساخت و اجرای بنچمارک (روی Host؛ برای Target با
#                     STM32H7 از DWT استفاده می‌شود)
#   make clean
#
# نکته‌ی Flash: حتی اگر همه‌ی src/*.c کامپایل شوند، ماژول‌های خاموش در
# dsp_config.h به صورت فایل خالی در می‌آیند و هیچ بایتی به باینری اضافه
# نمی‌کنند (Feature guards در سطح #if).
#
# برای پروژه‌های STM32H7 با CMSIS:
#   make DSP_USE_CMSIS=1  →  ماکروی -DDSP_USE_CMSIS_DSP=1 اضافه می‌شود
# ============================================================================

CC       ?= gcc
AR       ?= ar
CFLAGS   ?= -O2 -Wall -Wextra -std=c99
INC       = -Iinc
LIB       = build/libdsp.a
SRCS     := $(wildcard src/*.c)
OBJS     := $(SRCS:src/%.c=build/%.o)

ifeq ($(DSP_USE_CMSIS),1)
CFLAGS += -DDSP_USE_CMSIS_DSP=1
endif

.PHONY: all test bench clean

all: $(LIB)

build:
	mkdir -p build

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

test: all
	$(CC) $(CFLAGS) $(INC) -Itests tests/test_runner.c tests/test_core.c \
		tests/test_utils.c tests/test_adaptive.c tests/test_resample.c \
		$(SRCS) -lm -o build/test_all
	./build/test_all
	$(CC) $(CFLAGS) $(INC) -Itests tests/test_design.c $(SRCS) -lm -o build/test_design
	./build/test_design

bench: all
	$(CC) $(CFLAGS) $(INC) -Ibench bench/bench_main.c $(SRCS) -lm -o build/bench
	./build/bench

clean:
	rm -rf build
