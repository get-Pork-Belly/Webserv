#!/bin/bash

SERVER_RUN=0
QUIT=1

echo "> SET ID & PASSWORD FIRST!"
make pwgenerator
./htpassword_generator

res="${?}"
if [ ${res} -eq ${QUIT} ]; then
    echo ">> QUIT"
elif [ ${res} -eq ${SERVER_RUN} ]; then
    echo "======================="
    echo ">> PUT CONFIG FILE NAME"
    read config_file_name
    echo ">>> Servers will run with config flie - ${config_file_name}"
    echo ">> PUT PLUGIN NAMES"
    read plugin_names
    echo ">>> Servers will run with plugins - ${plugin_names}"
    sed "s/plugin_names/${plugin_names}/" ./config/${config_file_name} > ./config/.temp_config 
    make
    ./Webserv ./config/.temp_config
    rm ./config/.temp_config
fi