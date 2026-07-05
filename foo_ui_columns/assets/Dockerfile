FROM docker.io/archlinux:latest

RUN pacman --sync --refresh --noconfirm icoutils imagemagick inkscape npm nodejs pacman-contrib \
    && paccache --remove
