#!/bin/bash
g++ -o dbwrite *.cpp -L/usr/local/lib/mariadb -lmariadbcpp

echo "Compile Completed"