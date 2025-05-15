FROM docker.io/archlinux:latest

RUN pacman --sync --refresh --noconfirm imagemagick inkscape npm nodejs pacman-contrib \
    && paccache --remove
