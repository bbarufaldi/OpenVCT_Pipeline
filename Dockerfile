FROM nvcr.io/nvidia/nvhpc:24.3-devel-cuda12.3-ubuntu22.04

ARG DEBIAN_FRONTEND=noninteractive

#setup layers
RUN apt-get update && apt-get install -y \
    libgdcm-dev \
    python3.6 \
    python3-pip \
    git \
    wget \
    pocl-opencl-icd \
    ocl-icd-dev \
    ocl-icd-opencl-dev \
    ocl-icd-libopencl1 \
    opencl-headers \
    freeglut3-dev \
    libgl1-mesa-glx \
    libgl1-mesa-dri \
    mesa-utils \
    xvfb \
    clinfo \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

ENV PATH="${PATH}:app/helpers/VctToolkit/include:app/helpers/VctToolkit/ZipLib/include:app/helpers/VctToolkit/ZipLib/bin/x64/Release"

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES compute,utility

COPY . .

RUN pip install --no-cache-dir -r requirements.txt

