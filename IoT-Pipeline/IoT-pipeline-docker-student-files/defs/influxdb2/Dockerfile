# influxdb:2.4.0
# FROM influxdb:latest (14.09.2025 latest is 2.7.12)
# - version 3.x is based on a new architecture and 
#   may have different license and feature model
FROM influxdb:2.7.12

# install curl for heathcheck op
RUN apt-get update && apt-get install -y curl && rm -rf /var/lib/apt/lists/*
