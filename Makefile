# C++ 编译器
CXX = g++

# 编译选项：
# -std=c++11: 使用 C++11 标准
# -pthread: 支持多线程
# -I./include: 在我们自己的项目中寻找头文件 (例如 auth.h)
# -I$(SDK_PATH)/include: 在讯飞SDK的目录中寻找头文件 (例如 sparkchain.h)
CXXFLAGS = -std=c++11 -pthread -I./include -I$(SDK_PATH)/include

# 链接选项：
# **核心修正：已经从此行中移除了 -lwebsocketpp**
LDFLAGS = -L$(SDK_PATH)/libs -lboost_system -lssl -lcrypto -lpthread -lsqlite3 -lasound -lSparkChain

# --- 请在这里配置你的讯飞SDK实际存放路径 ---
# 注意：我们增加了一层重复的目录名
SDK_PATH = /home/bobac3/SparkChain_Linux_SDK_2.0.0_rc1/SparkChain_Linux_SDK_2.0.0_rc1

# 目标可执行文件名
TARGET = intelligent_player

# 所有源文件
SOURCES = src/intelligent_player.cpp src/auth.cpp src/spark_client.cpp src/tts_client.cpp src/mic_capture.cpp src/asr_client.cpp

# 根据源文件自动生成目标文件名 (例如 a.cpp -> a.o)
OBJECTS = $(SOURCES:.cpp=.o)

# 默认目标：编译所有
all: $(TARGET)

# 链接规则：如何从所有 .o 文件生成最终的程序
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# 编译规则：如何从一个 .cpp 文件生成一个 .o 文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则：删除所有生成的文件
clean:
	rm -f src/*.o $(TARGET)
