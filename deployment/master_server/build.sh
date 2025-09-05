#!/bin/bash

docker build --network host --build-arg HTTP_PROXY=http://127.0.0.1:3128 --build-arg HTTPS_PROXY=http://127.0.0.1:3128 -f Dockerfile -t 127.0.0.1:5000/control:module-V1.0-20201220 .