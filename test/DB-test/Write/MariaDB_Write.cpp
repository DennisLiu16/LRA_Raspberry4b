#include <iostream>
#include <mariadb/conncpp.hpp>

// Main Process
int main(int argc, char **argv)
{
   try {
      // Instantiate Driver
      sql::Driver* driver = sql::mariadb::get_driver_instance();

      // Configure Connection
      // The URL or TCP connection string format is
      // ``jdbc:mariadb://host:port/database``.
      sql::SQLString url("jdbc:mariadb://127.0.0.1:3306/LRA");

      // Use a properties map for the user name and password
      sql::Properties properties({
            {"user", "nthu"},
            {"password", "nthumems"}
         });

      // Establish Connection
      // Use a smart pointer for extra safety
      std::unique_ptr<sql::Connection> conn(driver->connect(url, properties));

      // Use Connection
      if(conn != nullptr)
	  	std::cout << "MariaDB Connection through C++ successfully" << std::endl;
      //TODO
      // try to write something
	  

      // Close Connection
      conn->close();
   }

   // Catch Exceptions
   catch (sql::SQLException& e) {
      std::cerr << "Error Connecting to the database: "
         << e.what() << std::endl;

      // Exit (Failed)
      return 1;
   }

   // Exit (Success)
   return 0;
}