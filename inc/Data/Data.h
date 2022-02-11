#ifndef _LRA_DATA_H_
#define _LRA_DATA_H_

// sys include
#include <mariadb/conncpp.hpp>

// c++ std libraries
#include <filesystem>
#include <vector>
#include <string>
#include <thread>

// c std libraries
extern "C"
{

}

//project include 
#include <PI/LRA_PI_Util.h>

namespace LRA_Data
{
    using namespace std;
    using namespace std::filesystem;
    class TXT
    {
        public:
            int err = 0;
            FILE* fd;

            /*Constructors*/
            TXT(string& path, string& file_name, const char* write_mode);

            /*Destructors*/
            ~TXT();

            /*Update*/
            void updatePath(string& path);

            /**
             * @brief open fd of txt from given path and file_name
             * 
             */
            void open();

            void close();
        
        protected:

        private:
            fs::path _full_path = "";
            fs::path _opened_path = "";
            char* _write_mode;
    };

    class MariaDB
    {
        public:
            /*vars*/
            unique_ptr<sql::Connection> conn;

            /*Constructors*/
            MariaDB();

            /*Destructors*/
            ~MariaDB();

        protected:

        private:
    };

    class Data : public TXT, public MariaDB
    {
        public:
            /*Constructors*/
            Data();

            /*Destructors*/
            ~Data();

        protected:

        private:
    };
}
#endif
