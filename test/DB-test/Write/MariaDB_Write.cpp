#include <iostream>
#include <mariadb/conncpp.hpp>
#include <time.h>

float getTime(timespec& t1, timespec& t2)
{
   return (t2.tv_sec-t1.tv_sec)*1000+(t2.tv_nsec-t1.tv_nsec)/1e6;
}

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
      std::unique_ptr<sql::Connection> conn2(driver->connect(url, properties));

      // Use Connection
      conn->setAutoCommit(false);
      conn2->setAutoCommit(false);
      if(conn != nullptr)
	  	std::cout << "MariaDB Connection through C++ successfully" << std::endl;

      // XXX
      // try to write something
      timespec t1,t2,t;
      clock_gettime(CLOCK_REALTIME,&t1);
      float total_cost = 0.0;
      int write_number = 800;
      int loop_number = 50;
      sql::PreparedStatement *prep_stmt = conn->prepareStatement("INSERT INTO test_acc(time,accX,accY,accZ) VALUES (?,?,?,?)");
      sql::PreparedStatement *prep_stmt2 = conn2->prepareStatement("INSERT INTO test_acc(time,accX,accY,accZ) VALUES (?,?,?,?)"); 
      for(int j = 0; j < loop_number; ++j)
      {
         int i = 0;
         
         printf("Write Data to DB for %d messages\n", write_number);
         
         while(i < write_number)
         {
               ++i;
               clock_gettime(CLOCK_REALTIME, &t);
               prep_stmt->setFloat(1,getTime(t1,t));
               prep_stmt->setFloat(2,0.375);
               prep_stmt->setFloat(3,0.233);
               prep_stmt->setFloat(4,-0.11);
               prep_stmt->addBatch();

               prep_stmt2->setFloat(1,getTime(t1,t));
               prep_stmt2->setFloat(2,2.55);
               prep_stmt2->setFloat(3,3.865);
               prep_stmt2->setFloat(4,-95.21);
               prep_stmt2->addBatch();

         }
            prep_stmt->executeBatch();
            prep_stmt2->executeBatch();
            conn->commit();
            conn2->commit();
            prep_stmt->clearBatch();
            prep_stmt2->clearBatch();
            clock_gettime(CLOCK_REALTIME,&t2);

            float time_cost = getTime(t1,t2);
            std::cout << "cost " << time_cost << " (ms)" <<std::endl;
            //delete prep_stmt;
            t1 = t2;
            total_cost += time_cost;
      }
      std::cout << "Average time = " << total_cost/loop_number << " (ms)" << std::endl;
      std::cout << "Total time = " << total_cost / 1000 << " (s)" << std::endl;
      
      // Close Connection
      delete prep_stmt;
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