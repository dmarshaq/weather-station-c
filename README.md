# Weather Station

A weather station is a system that measures and records atmospheric conditions to monitor and predict weather. It provides data such as temperature, humidity, air pressure, wind, and rainfall, which are essential for forecasting, research, and daily decision-making.

Table Of Contents
-----------------
* [Quick Start](#quick-start)
    * [Requirements](#requirements)
    * [How to Build](#how-to-build)
    * [How to Run](#how-to-run)
* [Features](#features)

Quick Start
-----------------
#### Requirements
* Git
* GCC - (GNU C Compiler)
* C Standard Library
* Docker - (If not using Linux)



#### How to Build
To build a project:

1. Open a terminal and navigate to your projects directory.
2. Clone the repository:
    ```bash
    git clone https://github.com/dmarshaq/weather-station-c.git
    ```
3. Enter the project.
    ```bash
    cd weather-station-c
    ```
4. Run the script to compile and run the program.
   ```bash
   ./run.sh
   ```
   - Or build docker image (If not using Linux).
   ```bash
   docker build -t weather-station-c .
   ```
    
#### How to Run
To run the project (If not using Linux):

1. Run the docker container.
    ```bash
    docker run --rm -it weather-station-c
    ```


Features
-----------------
- Live display of the data on the graph.
- Big data collection over long period of time into .csv file.
