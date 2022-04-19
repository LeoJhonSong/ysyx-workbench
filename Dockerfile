# image for ysyx project
FROM archlinux/archlinux:base-devel

ARG USER_NAME=student

# set password of root and add a sudo user 'student' with password ysyx
RUN echo "root:root" | chpasswd && \
    useradd --create-home --shell /bin/bash ${USER_NAME} && \
    echo "${USER_NAME}:ysyx" | chpasswd && \
    echo "${USER_NAME} ALL=(ALL) ALL" >> /etc/sudoers

# change to china mirrors and add archlinuxcn, then refresh source lists and install packages
RUN sed -i '1s/^/Server = https:\/\/mirrors.tuna.tsinghua.edu.cn\/archlinux\/\$repo\/os\/\$arch\n/' /etc/pacman.d/mirrorlist && \
    echo -e "[archlinuxcn]\nSigLevel = Never\nServer = https://mirrors.tuna.tsinghua.edu.cn/archlinuxcn/\$arch" | tee -a /etc/pacman.conf && \
    pacman -Syy && \
    pacman -S --noconfirm archlinuxcn-keyring \
        yay \
        openssh \
        gdb \
        llvm \
        cmake \
        clang \
        verilator \
        gtkwave \
        vi \
        neovim \
        python-pynvim \
        xclip \
        zsh \
        tmux \
        lsd \
        fd \
        bat \
        ranger \
        ccze \
        python-pip \
        noto-fonts \
        && \
    ln -s /usr/bin/nvim /usr/bin/vim && \
    pip install wakatime \
        compiledb \
        && \
    yay -S --noconfirm lazygit

# change user and dir
USER ${USER_NAME}
WORKDIR /home/${USER_NAME}

# copy all config files to ~
COPY --chown=${USER_NAME} configs .
RUN curl -fsSL https://raw.github.com/ohmyzsh/ohmyzsh/master/tools/install.sh -o /tmp/install.sh && \
    sh /tmp/install.sh --unattended --keep-zshrc && \
    rm /tmp/install.sh && \
    curl -L git.io/antigen > ~/antigen.zsh && \
    zsh -c "source ~/.zshrc" && \
    git clone --depth 1 https://github.com/junegunn/fzf.git ~/.fzf && \
    ~/.fzf/install --completion --no-bash --no-key-bindings --no-update-rc

RUN git config --global user.name "Leo@YSYX" && \
    git config --global user.email "LeoJhon.Song@outlook.com" && \
    git config --global credential.helper store && \
    git config --global http.proxy http://127.0.0.1:7890 && \
    echo -e "Host github.com\n\tStrictHostKeyChecking no\n" >> ~/.ssh/config