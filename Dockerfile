FROM fedora:32

WORKDIR /src/libnmprotocol

RUN dnf install -y \
        autoconf \
        automake \
        libtool \
        g++ \
        make \
        diffutils \
        flex \
        bison \
        tcl-devel \
        alsa-lib-devel \
    ; \
    dnf clean all;

COPY --from=4c0n/libpdl:latest /usr/local/include /usr/include
COPY --from=4c0n/libpdl:latest /usr/local/lib/libpdl.la /usr/lib/libpdl.la
COPY --from=4c0n/libpdl:latest /usr/local/lib/libpdl.a /usr/lib/libpdl.a

COPY --from=4c0n/libppf:latest /usr/local/include /usr/local/include
COPY --from=4c0n/libppf:latest /usr/local/lib/libppf.la /usr/local/lib/libppf.la
COPY --from=4c0n/libppf:latest /usr/local/lib/libppf.a /usr/local/lib/libppf.a

COPY --from=4c0n/libnmpatch:latest /usr/local/include /usr/include
COPY --from=4c0n/libnmpatch:latest /usr/local/lib/libnmpatch.la /usr/lib/libnmpatch.la
COPY --from=4c0n/libnmpatch:latest /usr/local/lib/libnmpatch.a /usr/lib/libnmpatch.a

COPY . /src/libnmprotocol

RUN set -eux; \
    aclocal; \
    libtoolize; \
    automake --add-missing && ls -lah; \
    autoreconf; \
    ./configure; \
    make; \
    make check; \
    make install;
