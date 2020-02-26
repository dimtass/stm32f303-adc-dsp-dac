#!/bin/bash -e

echo "Building the project in Linux environment using Docker"
docker run --rm -it -v `pwd`:/tmp -w=/tmp dimtass/stm32-cde-image:0.1 -c "./build.sh"