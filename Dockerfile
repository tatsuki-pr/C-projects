# Use an official Linux environment
FROM ubuntu:22.04

# Stop Ubuntu from asking Y/N questions during install
ENV DEBIAN_FRONTEND=noninteractive

# Install the C++ compiler, SFML library, and the virtual monitor tool
RUN apt-get update && apt-get install -y \
    g++ \
    libsfml-dev \
    xvfb \
    && rm -rf /var/lib/apt/lists/*

# Create a folder inside the container for our game
WORKDIR /app

# Copy all your local files (main.cpp, font.ttf) into the container
COPY . .

# Compile the game for Linux
RUN g++ main.cpp -o snake_game -lsfml-graphics -lsfml-window -lsfml-system

# Run the game inside the virtual monitor so it doesn't crash
CMD ["xvfb-run", "-a", "./snake_game"]