# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -fPIC -Icommon/include -Ilibqr/include -Iclient/include

# Директории
BIN_DIR = bin
LIB_DIR = lib
$(shell mkdir -p $(BIN_DIR) $(LIB_DIR))

# Флаги для библиотек
LIBQR_LIBS = -lpng -lqrencode -lpthread

# Флаги для Qt
QT_CFLAGS = $(shell pkg-config --cflags Qt5Widgets Qt5Network)
QT_LIBS = $(shell pkg-config --libs Qt5Widgets Qt5Network)

# Определяем какие файлы требуют Qt флагов
QT_SOURCES = $(wildcard client/src/*.cpp) common/src/network_utils.cpp

# Библиотека QR
LIBQR_SRCS = libqr/src/qr_generator.cpp common/src/logging.cpp
LIBQR_OBJ = $(LIBQR_SRCS:.cpp=.o)
LIBQR_LIB = $(LIB_DIR)/libqr.a

# Сервер
SERVER_SRCS = server/src/server.cpp
SERVER_OBJ = $(SERVER_SRCS:.cpp=.o)
SERVER_EXE = $(BIN_DIR)/qr_server

# Клиент
CLIENT_SRCS = client/src/client_gui.cpp client/src/main.cpp common/src/network_utils.cpp
CLIENT_OBJ = $(CLIENT_SRCS:.cpp=.o)
CLIENT_MOC = client/src/moc_client_gui.cpp
CLIENT_EXE = $(BIN_DIR)/qr_client

# Цели по умолчанию
all: $(LIBQR_LIB) $(SERVER_EXE) $(CLIENT_EXE)

# Сборка библиотеки QR
$(LIBQR_LIB): $(LIBQR_OBJ)
	ar rcs $@ $^

# Сборка сервера
$(SERVER_EXE): $(SERVER_OBJ) $(LIBQR_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L$(LIB_DIR) -lqr $(LIBQR_LIBS)

# Сборка клиента
$(CLIENT_EXE): $(CLIENT_OBJ) $(CLIENT_MOC) $(LIBQR_LIB)
	$(CXX) $(CXXFLAGS) $(QT_CFLAGS) -o $@ $^ -L$(LIB_DIR) -lqr $(QT_LIBS)

# Генерация moc-файла
$(CLIENT_MOC): client/include/client_gui.h
	moc $< -o $@

# Правило для .cpp -> .o с Qt флагами для определенных файлов
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(if $(filter $(QT_SOURCES),$<),$(QT_CFLAGS)) -c -o $@ $<

# Очистка
clean:
	rm -f $(SERVER_OBJ) $(SERVER_EXE) \
	      $(CLIENT_OBJ) $(CLIENT_EXE) $(CLIENT_MOC) \
	      $(LIBQR_OBJ) $(LIBQR_LIB) \
	      common/src/*.o libqr/src/*.o
	rmdir $(BIN_DIR) $(LIB_DIR) 2>/dev/null || true

.PHONY: all clean
