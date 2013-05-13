# 编译选项
CC = gcc
CFLAGS = -g -O2 -Wall -fPIC --shared

# 把所有以.c结尾的文件放到列表里
SOURCES = $(wildcard *.c)
# 匹配sources列表里所有.c文件替换成.o文件放到列表里
OBJS = $(patsubst %.c,%.so,$(SOURCES))

all: $(OBJS)

# 生成动态库文件
$(OBJS): %.so: %.c
	$(CC) $(OBJS) $< -o $@

# 删除.so文件
clean:
	rm $(OBJS) -f
