apt-get update
apt-get install -y --no-install-recommends \
    build-essential \
    python3.11 \
    python3.11-venv \
    libboost-all-dev \
    libfftw3-dev \
    libomp-dev \
    libtbb-dev \
    cmake \
    ninja-build \

python3.11 -m venv ./venv
. ./venv/bin/activate
curl -sS https://bootstrap.pypa.io/get-pip.py | python3.11
pip install -r ./scripts/py/requirements.txt
