FROM gcc:latest

WORKDIR /mulisse

# Copy only the setup script and requirements file first
COPY ./scripts/sh/setup.sh ./scripts/sh/setup.sh
COPY ./scripts/py/requirements.txt ./scripts/py/requirements.txt

# Install dependencies
RUN chmod +x ./scripts/sh/setup.sh && \
    ./scripts/sh/setup.sh

# Copy only the source code and the builder script
COPY ./src ./src
COPY ./lib ./lib
COPY ./tests ./tests
COPY ./extern ./extern
COPY ./CMakeLists.txt ./CMakeLists.txt
COPY ./scripts/sh/build.sh ./scripts/sh/build.sh

# Build the project
RUN chmod +x ./scripts/sh/build.sh && \
    ./scripts/sh/build.sh

# Copy the rest of the files
COPY . .

# Overwrite `local_settings.json` with `docker_settings.json`
# NOTE: the dataset files have to be mounted to the path specified in `docker_settings.json`, using:
#       `docker run -v /local_path/to/dataset:/container_path/to/dataset ...`
RUN mv ./scripts/docker_settings.json ./scripts/local_settings.json

# Define the entrypoint and default command
ENTRYPOINT ["/mulisse/scripts/sh/entrypoint.sh"]
CMD ["-i", "/mulisse/scripts/run_configs/default_config.json"]

# Define a volume for the LOGS directory
VOLUME ["/mulisse/LOGS"]