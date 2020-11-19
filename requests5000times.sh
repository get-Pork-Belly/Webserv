#!/bin/bash

echo "Test start!"
for ((i = 0; i < 5000; i++)); do
	curl -X GET localhost:8080/directory
done
echo "Test finish!"
