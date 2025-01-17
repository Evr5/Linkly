UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CORES := $(shell nproc)
else
	CORES := 1
endif

BUILD_DIR = build
OUTPUT_DIR = .

all:
	@cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$(abspath $(OUTPUT_DIR))
	@cmake --build $(BUILD_DIR) -- -j$(CORES)

debug:
	@cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$(abspath $(OUTPUT_DIR))
	@cmake --build $(BUILD_DIR) -- -j$(CORES)

clean:
	@rm -rf $(BUILD_DIR) $(OUTPUT_DIR)/serveur-chat $(OUTPUT_DIR)/chat 

re: clean all

re_debug: clean debug

.PHONY: all debug clean re re_debug
