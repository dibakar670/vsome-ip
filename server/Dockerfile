FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies (including gcc-9/g++-9 and tools)
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-9 g++-9 build-essential cmake git pkg-config \
    libboost-all-dev libsystemd-dev libcap-dev \
    libtinyxml2-dev libglib2.0-dev && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90 && \
    rm -rf /var/lib/apt/lists/*

# Step 2: Copy your full project (including vsomeip)
# -------------------------------
WORKDIR /app
COPY . /app

# -------------------------------
# Step 3: Patch -Werror (optional but recommended)
# -------------------------------
RUN sed -i 's/-Werror//g' /app/vsomeip/CMakeLists.txt

# -------------------------------
# Step 4: Build and install vsomeip
# -------------------------------
WORKDIR /app/vsomeip/build
RUN cmake .. && make -j$(nproc) && make install

# -------------------------------
# Step 5: Build your application
# -------------------------------
# Step 4: Build and install vsomeip
WORKDIR /app/vsomeip
RUN mkdir -p build
WORKDIR /app/vsomeip/build
RUN cmake .. && make -j$(nproc) && make install

# -------------------------------
# Step 6: Runtime environment
# -------------------------------
ENV VSOMEIP_CONFIGURATION=/app/vsomeip-config.json
ENV LD_LIBRARY_PATH=/usr/local/lib
ENV VSOMEIP_APPLICATION_NAME=hello_server



RUN mkdir -p /app/build
WORKDIR /app/build
RUN cmake .. && make -j$(nproc)

# -------------------------------
# Step 7: Run your app (adjust if needed)
# -------------------------------
CMD ["./build/myapp"]