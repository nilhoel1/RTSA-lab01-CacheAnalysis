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
    fish \
    zsh


