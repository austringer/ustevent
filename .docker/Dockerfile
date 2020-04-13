FROM ubuntu:18.04 AS base

ARG DEBIAN_FRONTEND=noninteractive

# replace wiht a mirroring repository in China
RUN mv /etc/apt/sources.list /etc/apt/sources.list.bak \
    # 默认注释了源码镜像以提高 apt update 速度，如有需要可自行取消注释
    && echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic main restricted universe multiverse" > /etc/apt/sources.list \
    # && echo "deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list \
    # && echo "deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-updates main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list \
    # && echo "deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-backports main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list \
    # && echo "deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-security main restricted universe multiverse" >> /etc/apt/sources.list \
    # 预发布软件源，不建议启用
    # && echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list \
    # && echo "deb-src http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ bionic-proposed main restricted universe multiverse" >> /etc/apt/sources.list \
    && apt-get update \
    && apt-get -y upgrade \
    && apt-get install -y --no-install-recommends \
        apt-utils \
        apt-transport-https \
        ca-certificates  \
        locales \
        curl \
        dirmngr \
        procps \
        nano \
        zlib1g \
        bzip2 \
        libunwind8 \
    && apt-get clean && rm -rf /tmp/* /var/tmp/* && rm -rf /var/lib/apt/lists/*


FROM base AS development_base

ARG PythonMajorVersion=3
ARG PythonMinorVersion=6
ENV PYTHON_MAJOR_VERSION=${PythonMajorVersion} \
    PYTHON_MINOR_VERSION=${PythonMinorVersion}
# explicitly assign PYTHONPATH for some package like protobuf
ENV LOCAL_PYTHONPATH="/usr/local/lib/python${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION}/dist-packages"
ENV PYTHONPATH="${LOCAL_PYTHONPATH}:${PYTHONPATH}"

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        git \
        git-lfs \
        pkgconf \
        autoconf \
        automake \
        libtool \
        make \
        cmake \
        ccache \
        distcc \
        g++ \
        gdb \
        gdbserver \
        # clang clang-tools lldb lld llvm \
        python3 \
        python3-dev \
        python3-pip \
        libc-dev \
        libssl-dev \
        libunwind-dev \
    && apt-get clean && rm -rf /tmp/* /var/tmp/* && rm -rf /var/lib/apt/lists/*


FROM development_base AS development_boost

ARG BoostVersion
ARG BoostDirectory
ENV BOOST_VERSION ${BoostVersion}
ENV BOOST_DIRECTORY ${BoostDirectory}

# Add file if exists.
ADD .docker/Dockerfile third_party/archive/${BoostDirectory}.tar.bz2* /
RUN if [ ! -d ${BOOST_DIRECTORY} ]; then curl -L https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/${BOOST_DIRECTORY}.tar.bz2 | tar jx; fi \
    && cd ${BOOST_DIRECTORY} \
    && ./bootstrap.sh --with-libraries=fiber --prefix=/usr/local \
    && ./b2 -j $(nproc) debug-symbols=on install

# FROM development_base AS development_protobuf

# ARG ProtobufVersion
# ENV PROTOBUF_VERSION ${ProtobufVersion}

# # Add file if exists.
# ADD .docker/Dockerfile third_party/archive/protobuf-all-${ProtobufVersion}.tar.gz* /
# RUN if [ ! -d protobuf-${PROTOBUF_VERSION} ]; then curl -L https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-all-${PROTOBUF_VERSION}.tar.gz | tar zx; fi

# RUN cd protobuf-${PROTOBUF_VERSION} \
#     && ./configure --prefix=/usr/local \
#     # By default, the protobuf is compiled with "-O2 -g -DNDEBUG".
#     && make -j $(nproc) \
#     # && make check \
#     && make install \
#     && cd .. \
#     # Python support
#     && cd protobuf-${PROTOBUF_VERSION}/python \
#     && python3 setup.py build --cpp_implementation \
#     # && python3 setup.py test --cpp_implementation \
#     && python3 setup.py install --cpp_implementation --prefix=/usr/local \
#     && cd ../.. && rm -rf protobuf-${PROTOBUF_VERSION}

FROM development_base AS development

ARG Project
ENV PROJECT ${Project}
ENV CCACHE_DIR /workspace/.ccache

COPY --from=development_boost /usr/local /usr/local
#COPY --from=development_protobuf /usr/local /usr/local

RUN ldconfig \
    && mkdir /${PROJECT}
