#
# Copyright (C) 2009 Niek Linnenbank
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#
# General variables.
#
TOPDIR	    := $(subst /tools/rules.mk,,$(lastword $(MAKEFILE_LIST)))
# $表示运行一个Makefile中的函数
# 根目录， 定义了 Linux 内核源代码所在的根目录。例如，各个子目录下的 Makefile 通过 $(TOPDIR)/Rules.make 就可以找到 Rules.make 的位置。 
# subst(a,b,c)，字符串替换函数，将c中的全部a替换成b
# $(MAKEFILE_LIST)会返回最近使用的Makefile文件，
# 如果使用了include 包含了一个Makefile文件那么返回就是这个被include 包含的Makefile文件的路径，
# 如果没有使用include 那么返回的是正在使用的Makefile文件所在的绝对路径，在这里就是/tools/rules.mk
# lastword这个函数表示提取最后一个MAKEFILE_LIST列表里的最后一个元素。元素与元素之间是以空格符分开。 $(lastword $(MAKEFILE_LIST)) 表示提取最后一个Makefile
# 这句就是TOPDIR=空
TOPDIR	    := ./$(subst tools/rules.mk,,$(TOPDIR))
# 这句就是TOPDIR=./
# = 是最基本的赋值
# := 是覆盖之前的值
# ?= 是如果没有被赋值过就赋予等号后面的值
# += 是添加等号后面的值
#
# Compiler chain.
#
# CROSS未定义变量，使用的话和没用一样
CC	    := $(CROSS)gcc
C++	    := $(CROSS)g++
CPP	    := $(CROSS)cpp
LD          := $(CROSS)ld
AR	    := $(CROSS)ar
RANLIB	    := $(CROSS)ranlib
CPPFLAGS    += -isystem $(TOPDIR)/include $(foreach inc,$(includes),-isystem $(TOPDIR)/$(inc))
# $(foreach <var>,<list>,<text>) 
# 这个函数的意思是，把参数<list>;中的单词逐一取出放到参数<var>;所指定的变量中，
# 然后再执行<text>;所包含的表达式。
# 每一次<text>;会返回一个字符串，循环过程中，<text>;的所返回的每个字符串会以空格分隔，
# 最后当整个循环结束时，<text>;所返回的每个字符串所组成的整个字符串（以空格分隔）将会是foreach函数的返回值。
# -isystem 是gcc的编译选项
CFLAGS	    += $(CPPFLAGS) -Wall -Werror -MD -O0 -g3 -fno-builtin $(cflags)
C++FLAGS    += $(CFLAGS) -fno-rtti -fno-exceptions
ASMFLAGS    += $(CPPFLAGS) -Wall -Werror -O0 -g3
LDFLAGS     += --whole-archive -nostdlib -nodefaultlibs $(ldflags)
ldscript    ?= $(TOPDIR)/tools/link.ld
LDSCRIPT     = $(ldscript)

#
# Source and object files.
#
C_SOURCES   += $(wildcard *.c)
# 在Makefile规则中，通配符会被自动展开。
# 但在变量的定义和函数引用时，通配符将失效。
# 这种情况下如果需要通配符有效，就需要使用函数“wildcard”，
# 它的用法是：$(wildcard PATTERN...) 。
# 在Makefile中，它被展开为已经存在的、使用空格分开的、匹配此模式的所有文件列表。
# 如果不存在任何符合此模式的文件，函数会忽略模式字符并返回空。
# 这句话会将当前目录的所有.c文件加入到C_SOURCES后面
C_OBJECTS   += $(patsubst %.c,%.o,$(C_SOURCES))
# patsubst是个模式替换函数
# 格式：$(patsubst <pattern>,<replacement>,<text> ) 
# 功能：查找<text>中的单词（单词以“空格”、“Tab”或“回车”“换行”分隔）是否符合模式<pattern>，如果匹配的话，则以<replacement>替换。
# 这里，<pattern>可以包括通配符“%”，表示任意长度的字串。
# 如果<replacement>中也包含“%”，那么，<replacement>中的这个“%”将是<pattern>中的那个“%”所代表的字串。（可以用“\”来转义，以“\%”来表示真实含义的“%”字符）
# 返回：函数返回被替换过后的字符串。
C++_SOURCES += $(wildcard *.cpp)
C++_OBJECTS += $(patsubst %.cpp,%.o,$(C++_SOURCES))
ASM_SOURCES += $(wildcard *.S)
ASM_OBJECTS += $(patsubst %.S,%.o,$(ASM_SOURCES))
SOURCES	    += $(C_SOURCES) $(C++_SOURCES) $(ASM_SOURCES)
OBJECTS	    += $(C_OBJECTS) $(C++_OBJECTS) $(ASM_OBJECTS)

#
# Process library dependencies.
#
CFLAGS      += $(foreach lib,$(libraries),-include $(TOPDIR)/lib/$(lib)/Default.h -isystem $(TOPDIR)/lib/$(lib)/)
LDFLAGS	    += $(foreach lib,$(libraries), $(TOPDIR)/lib/$(lib)/$(lib).a)

#
# The default target is all.
#
# 用冒号定义的是目标变量
all: pre-hooks subdirs $(OBJECTS) $(program) $(library) post-hooks
pre-hooks:
post-hooks:

#
# Recurse into subdirectories (NOT parallel!)
#
subdirs:
	@for dir in $(subdirs); do \
	  $(MAKE) -C $$dir; \
	done

#
# Link a program.
#
$(program): $(OBJECTS)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@ $(OBJECTS) $(objects)
	$(LD) $(LDFLAGS) -T $(LDSCRIPT) -o $@.bin --oformat binary $(OBJECTS) $(objects)

#
# Built a library.
#
$(library): $(OBJECTS)
	$(AR) ru $(library) $(OBJECTS) $(objects)
	$(RANLIB) $(library)

#
# C program source.
#
$(C_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

#
# C++ program source.
#
$(C++_OBJECTS) : %.o : %.cpp
	$(C++) $(C++FLAGS) -o $@ -c $<

#
# Assembly program source.
#
$(ASM_OBJECTS) : %.o : %.S
	$(CC) $(ASMFLAGS) -o $@ -c $<

#
# Include generated dependancies.
#
-include $(wildcard *.d)

#
# Cleans up OBJECTS.
#
clean:
	@for dir in $(subdirs); do \
	  $(MAKE) -C $$dir clean; \
	done
	rm -f $(OBJECTS) $(program) $(program).bin $(library) *.d $(clean) $(CLEAN)

#
# These need to be forced.
#
.PHONY: clean $(subdirs) $(PHONY)
