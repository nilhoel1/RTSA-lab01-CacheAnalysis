FROM archlinux


# 1. INSTALL DEPENDENCIES
RUN pacman -Syu --noconfirm \
    git \
    cmake \
    ninja \
    gcc \
    llvm \
    clang \
    gdb \
    lldb \
    python-pip \
    fish \
    zsh

# 2. INSTALL LIT
RUN pip3 install lit



