#!/bin/bash
g++ -o dbconnection MariaDB_Connection.cpp -L/usr/local/lib/mariadb -lmariadbcpp

echo "Compile Completed"