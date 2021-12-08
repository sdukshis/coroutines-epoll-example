FROM gcc:11.2

RUN apt update -y && apt install -y \
    git \
    cmake \
    netcat \
    curl

ENV RUSTUP_HOME=/opt/rust \
    CARGO_HOME=/opt/rust \
    PATH=/opt/rust/bin:$PATH
RUN curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf -o rustup.sh && sh rustup.sh -y --no-modify-path

WORKDIR /usr/src/app

ENTRYPOINT ["bash"]
