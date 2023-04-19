CC          := gcc
CFLAGS      := -g -Wall
LDLIBS      := -lreadline

TARGET      := slash

SRC_DIR     := src
BUILD_DIR   := obj

SRCS        := $(shell find $(SRC_DIR) -name *.c)
OBJS        := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRCS:.c=.o))

#--------------------------------------#

all: directory $(TARGET)

directory:
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) $(BUILD_DIR)


# RAPPELS
# $@ nom cible
# $< nom premiere dépendance
# $^ liste dépendance
