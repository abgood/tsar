# 编译选项
CC = gcc
CFLAGS = -g -O2 -Wall

# 库
LIBDL = -ldl
# 动态库动态加载
DYNAM = -rdynamic
LIBMYSQL = `mysql_config --libs`

# 编译子目录
DIRS = modules

# 执行文件
TARGET = tsar

# 所有的c文件生成o文件
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 把所有以.c结尾的文件放到列表里
SOURCES = $(wildcard *.c)
# 匹配sources列表里所有.c文件替换成.o文件放到列表里
OBJS = $(patsubst %.c,%.o,$(SOURCES))

# 生成执行文件
$(TARGET):$(OBJS)
	make -C $(DIRS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBDL) $(DYNAM) $(LIBMYSQL)
	chmod u+x $(TARGET)

# 删除.o文件和执行文件
clean:
	rm $(OBJS) $(TARGET) -f
	cd $(DIRS); make clean; cd ..
