#!/bin/bash

echo "Test start!"
for ((i = 0; i < 5000; i++)); do
	curl -X POST localhost:8080/directory/youpi.bla -d "hello"
done
echo "Test finish!"
