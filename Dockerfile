# Base Debian image
FROM debian:12

# Install dev tools
RUN apt-get update && apt-get install -y && apt-get install -y dos2unix \
    build-essential \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /weather-station-c

# Copies run script and executes it to build and run
COPY nob.c /weather-station-c/nob.c
COPY nob.h /weather-station-c/nob.h
COPY src /weather-station-c/src
COPY run.sh /weather-station-c/run.sh

RUN find . -type f \( -name "*.sh" -o -name "*.c" -o -name "*.h" \) -exec dos2unix {} +




# Gives exec permission to run.sh
RUN chmod +x /weather-station-c/run.sh


# Run the script
ENTRYPOINT ["/weather-station-c/run.sh"]




