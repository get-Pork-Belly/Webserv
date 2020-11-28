#!/bin/bash

echo "Test start!"
for ((i = 0; i < 5000; i++)); do
	curl -X GET localhost:8080
done
echo "Test finish!"
