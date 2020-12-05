#!/bin/bash

SERVER_RUN=0
QUIT=1

echo "> SET ID & PASSWORD FIRST!"
make pwgenerator
./htpassword_generator

res="${?}"
if [ ${res} -eq ${QUIT} ]; then
    echo "quit"
elif [ ${res} -eq ${SERVER_RUN} ]; then
    echo "======================="
    echo "> PUT CONFIG FILE NAME"
    read config_file_name
    echo "> Servers will run with ${config_file_name}"
    make
    ./Webserv ./config/${config_file_name}

else
    echo ${res}
fi

