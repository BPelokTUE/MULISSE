FROM gcc:latest

WORKDIR /mulisse

COPY . .

# Install dependencies
RUN chmod +x ./scripts/sh/setup.sh && \
    ./scripts/sh/setup.sh

# Build the project
RUN chmod +x ./scripts/sh/build.sh && \
    ./scripts/sh/build.sh

RUN chmod +x ./scripts/sh/run.sh

CMD ["./scripts/sh/run.sh"]
