#!/bin/bash

docker run -p 8082:80 -p 6002:6002 --env HTTP_PROXY=http://127.0.0.1:3128 --env HTTPS_PROXY=http://127.0.0.1:3128 -it 127.0.0.1:5000/control:module-V1.0-20201220 /bin/bash