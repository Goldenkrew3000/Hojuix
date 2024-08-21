# MODIFIED To remove all non-x86 architecture stuff
# This makefile is too confusing

# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := kernel

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

# Target architecture to build for. Default to x86_64.
override DEFAULT_KARCH := x86_64
$(eval $(call DEFAULT_VAR,KARCH,$(DEFAULT_KARCH)))

# User controllable C compiler command.
override DEFAULT_KCC := clang
$(eval $(call DEFAULT_VAR,KCC,$(DEFAULT_KCC)))

# Check if KCC is Clang.
override KCC_IS_CLANG := no
ifeq ($(shell $(KCC) --version 2>&1 | grep -i 'clang' >/dev/null 2>&1 && echo 1),1)
    override KCC_IS_CLANG := yes
endif

# User controllable linker command.
override DEFAULT_KLD := ld.lld
$(eval $(call DEFAULT_VAR,KLD,$(DEFAULT_KLD)))

# User controllable C flags.
override DEFAULT_KCFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,KCFLAGS,$(DEFAULT_KCFLAGS)))

# User controllable C preprocessor flags. We set none by default.
override DEFAULT_KCPPFLAGS :=
$(eval $(call DEFAULT_VAR,KCPPFLAGS,$(DEFAULT_KCPPFLAGS)))

ifeq ($(KARCH),x86_64)
    # User controllable nasm flags.
    override DEFAULT_KNASMFLAGS := -F dwarf -g
    $(eval $(call DEFAULT_VAR,KNASMFLAGS,$(DEFAULT_KNASMFLAGS)))
endif

# User controllable linker flags. We set none by default.
override DEFAULT_KLDFLAGS :=
$(eval $(call DEFAULT_VAR,KLDFLAGS,$(DEFAULT_KLDFLAGS)))

# Internal C flags that should not be changed by the user.
override KCFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections





# C PREPROCESSOR FLAGS (CHANGE INCLUDE DIRS HERE)
override KCPPFLAGS := \
    -L ../libc \
    -I ../libc/include \
    -I src/include \
    -I src/ \
    $(KCPPFLAGS) \
    -isystem freestanding-headers \
    -MMD \
    -MP





ifeq ($(KARCH),x86_64)
    # Internal nasm flags that should not be changed by the user.
    override KNASMFLAGS += \
        -Wall
endif

# SET X86 BUILD FLAGS
ifeq ($(KARCH),x86_64)
    ifeq ($(KCC_IS_CLANG),yes)
        override KCC += \
            -target x86_64-elf
    endif
    override KCFLAGS += \
        -m64 \
        -march=x86-64 \
        -mno-80387 \
        -mno-mmx \
        -mno-sse \
        -mno-sse2 \
        -mno-red-zone \
        -mcmodel=kernel
    override KLDFLAGS += \
        -m elf_x86_64
    override KNASMFLAGS += \
        -f elf64
else
    $(error Architecture $(KARCH) not supported)
endif





# Internal linker flags that should not be changed by the user. LINK FONT HERE
override KLDFLAGS += \
    -nostdlib \
    -lk \
    -static \
    -z max-page-size=0x1000 \
    -gc-sections \
    -T linker-$(KARCH).ld \
    -L ../libc \
    unifont.o  # Link font to kernel







# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell cd src && find -L * -type f -name '*.c' | LC_ALL=C sort) # *.c
override ASFILES := $(shell cd src && find -L * -type f -name '*.S' | LC_ALL=C sort) # *.S
ifeq ($(KARCH),x86_64)
override NASMFILES := $(shell cd src && find -L * -type f -name '*.asm' | LC_ALL=C sort) # *.asm
endif
override OBJ := $(addprefix obj-$(KARCH)/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o))
ifeq ($(KARCH),x86_64)
override OBJ += $(addprefix obj-$(KARCH)/,$(NASMFILES:.asm=.asm.o))
endif
override HEADER_DEPS := $(addprefix obj-$(KARCH)/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Ensure the dependencies have been obtained.
#override MISSING_DEPS := $(shell if ! test -d freestanding-headers || ! test -f src/cc-runtime.c || ! test -f src/limine.h; then echo 1; fi)
#ifeq ($(MISSING_DEPS),1)
#    $(error Please run the ./get-deps script first)
#endif

# Default target.
.PHONY: all
all: bin-$(KARCH)/$(OUTPUT)

# Link rules for the final executable.
bin-$(KARCH)/$(OUTPUT): GNUmakefile linker-$(KARCH).ld $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(KLD) $(OBJ) $(KLDFLAGS) -o $@

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
obj-$(KARCH)/%.c.o: src/%.c GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
obj-$(KARCH)/%.S.o: src/%.S GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(KCC) $(KCFLAGS) $(KCPPFLAGS) -c $< -o $@

ifeq ($(KARCH),x86_64)
# Compilation rules for *.asm (nasm) files.
obj-$(KARCH)/%.asm.o: src/%.asm GNUmakefile
	mkdir -p "$$(dirname $@)"
	nasm $(KNASMFLAGS) $< -o $@
endif

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin-$(KARCH) obj-$(KARCH)

# Remove everything built and generated including downloaded dependencies.
.PHONY: distclean
distclean:
	rm -rf bin-* obj-*

# Install the final built executable to its final on-root location.
.PHONY: install
install: all
	install -d "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	install -m 644 bin-$(KARCH)/$(OUTPUT) "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)-$(KARCH)"
