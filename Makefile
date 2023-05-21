EXECUTABLE := hoverclock

CC := gcc

DEPENDENCIES = xcb xcb-shape pangocairo

CFLAGS := $(shell pkg-config --cflags $(DEPENDENCIES)) -fno-exceptions -fshort-enums
LIBS   := $(shell pkg-config --libs $(DEPENDENCIES))

SOURCE_DIR := source
OBJECT_DIR := obj

SOURCE_FILES := $(shell find $(SOURCE_DIR) -name "*.c")
OBJECT_FILES := $(patsubst $(SOURCE_DIR)/%.c,$(OBJECT_DIR)/%.o,$(SOURCE_FILES))

all: $(OBJECT_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECT_FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECT_DIR):
	mkdir -p $(OBJECT_DIR)

clean:
	rm -rf $(OBJECT_DIR) $(EXECUTABLE)
