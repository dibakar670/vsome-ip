FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-9 g++-9 build-essential cmake pkg-config \
    libboost-all-dev libsystemd-dev libcap-dev \
    libtinyxml2-dev libglib2.0-dev && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy everything inside client folder (your current dir)
COPY . /app

# Optional: Patch -Werror in vsomeip CMakeLists.txt if present
RUN sed -i 's/-Werror//g' /app/vsomeip/CMakeLists.txt || true

# Build vsomeip library
WORKDIR /app/vsomeip/build
RUN cmake .. && make -j$(nproc) && make install

# Build your client app
WORKDIR /app
RUN mkdir -p build
WORKDIR /app/build
RUN cmake .. && make -j$(nproc)

ENV VSOMEIP_CONFIGURATION=/app/vsomeip-config.json
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV VSOMEIP_APPLICATION_NAME=hello_client

CMD ["./build/myclient2"]
