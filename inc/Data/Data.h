#ifndef _LRA_DATA_H_
#define _LRA_DATA_H_

// sys include
#include <mariadb/conncpp.hpp>

// c++ std libraries
#include <filesystem>
#include <vector>
#include <string>
#include <thread>
#include <tuple>
#include <stdexcept>

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
    namespace fs = std::filesystem;
    class TXT
    {
        public:
            FILE* fd;

            /*Constructors*/
            TXT(string& path, string& file_name, const char* write_mode, bool auto_reopen);

            /*Destructors*/
            ~TXT();

            /*Update*/
            void updatePath(string& nPath);

            void updateFileName(string& nFilename);

            /*Open*/
            void open(fs::path&, const char*);  // default argument need to know at compile time

            void open();

            /*Parse*/
            std::tuple<fs::path, fs::path> parse2_path_name(const char*);

            /*Close*/
            void close();
        
        protected:
            /*Form path*/
            /**
             * @brief include path valid check
             * 
             * @param path 
             * @param filename 
             * @return fs::path 
             */
            fs::path formPath(string& path, string& filename);

        private:
            string _path;
            string _filename;
            fs::path _full_path = "";
            fs::path _opened_path = "";
            char* _write_mode;
            bool _automode;
            
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
