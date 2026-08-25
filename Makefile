# ------------------------------------------------------------------
# Makefile for raylib DRM (native, no X11/Wayland) project
# Target: Raspbian / Debian Bookworm
# ------------------------------------------------------------------

# --- Project ---------------------------------------------------------------
PROJECT_NAME := pokajan
BUILD_DIR    := build
SRC_DIR      := src

# --- Toolchain ---------------------------------------------------------------
# Pinned to gcc since raylib's DRM build and this project target Raspbian
# specifically. Override on the command line if needed, e.g.:
#   make CC=arm-linux-gnueabihf-gcc   (cross-compiling)
#   make CC=clang                    (testing a different compiler)
CC      := gcc
RM      := rm -rf
MKDIR_P := mkdir -p

# --- Sources -----------------------------------------------------------------
# Recursively picks up every .c file under src/. New files are included
# automatically on the next build -- no need to edit this Makefile.
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRC := $(call rwildcard,$(SRC_DIR)/,*.c)

# Objects mirror the src/ tree inside build/
OBJ := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

# --- Raylib (statically built, DRM/eglfs platform) ---------------------------
RAYLIB_DIR := ../raylib/src

# --- Include / Library paths ---------------------------------------------------
INCLUDES := -I$(SRC_DIR) -I$(RAYLIB_DIR) -I/opt/vc/include
LDFLAGS  := -L$(RAYLIB_DIR) -L/opt/vc/lib

# --- Libraries -----------------------------------------------------------------
# Note: on Bookworm the Broadcom /opt/vc VideoCore stack (vcos/vchiq) is
# generally gone in favor of the Mesa V3D driver. If linking fails on
# -lvcos/-lvchiq_arm, just remove them from LDLIBS below.
LDLIBS := -lraylib -lm -lpthread -lGLESv2 -lEGL -lgbm -ldrm

# --- Compiler flags --------------------------------------------------------
CFLAGS  ?= -Wall -Wextra -std=gnu99 -MMD -MP
CFLAGS  += $(INCLUDES) -DPLATFORM_DRM

# Build type: use `make BUILD=debug` or `make BUILD=release` (default)
BUILD ?= release

ifeq ($(BUILD),debug)
	CFLAGS += -g -O0 -DDEBUG
else
	CFLAGS += -O2 -DNDEBUG
endif

# --- Targets -----------------------------------------------------------------
.PHONY: all clean rebuild run print-%

all: $(PROJECT_NAME)

$(PROJECT_NAME): $(OBJ)
	@echo "LD   $@"
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

# Compile each .c into build/, creating subdirectories as needed
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P) $(dir $@)
	@echo "CC   $<"
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

run: all
	./$(PROJECT_NAME)

clean:
	$(RM) $(BUILD_DIR) $(PROJECT_NAME)

rebuild: clean all

# Debug helper: `make print-CFLAGS`, `make print-LDLIBS`, etc.
print-%:
	@echo '$*=$($*)'