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
COPY ./docker_settings.json ./local_settings.json

# Build the project
RUN chmod +x ./scripts/sh/build.sh && \
    ./scripts/sh/build.sh

# Copy the rest of the files
COPY . .
RUN mv ./docker_settings.json ./local_settings.json

# Define the entrypoint and default command
ENTRYPOINT ["/mulisse/scripts/sh/entrypoint.sh"]
CMD ["-i", "/mulisse/scripts/run_configs/default_config.json"]

# Define a volume for the LOGS directory
VOLUME ["/mulisse/LOGS"]