#include <Data/Data.h>

using namespace LRA_Data;
using namespace LRA_PI_Util;
namespace fs = std::filesystem;

/*TXT region*/
TXT::TXT(string& path, string& file_name, const char* write_mode)
{
    // path valid 
    try
    {
        fs::path p{path};
        if(!fs::exists(p))
            // TODO:thorw a filesystem related error 

        // file_name existed or not  
        p /= file_name; 
        if(!fs::exists(p))
            print("Create new txt file name : %s", file_name);
        
        // write_mode valid
        if()
        {

        }
        // add extension .txt

        // get full path
        _full_path = p;
    }catch(fs::filesystem_error const& ex)
    {
        print("given path %s doesn't exist, please check again\n");
    }


}

TXT::~TXT()
{
    if(fd != nullptr) // close fd
        close();
}

void TXT::updatePath(string& path)
{

}

void TXT::open()
{
    
    if(fd != nullptr) // already opened
    {
        // TODO : && fd == _full_path
        //
        print("Already opened a txt file, please use close funtion first\n");
        return;

        else 
        {
            //TODO : reopen maybe 
        }
    }
    

    try // try to open file
    {
        fd = fopen(_full_path.c_str(), _write_mode);
        if(fd == nullptr)
            // XXX : exception case
            throw "OPEN FAILED";
        
        print("Open txt file at : %s sucessed", _full_path)
    }

    catch(const string& serr)
    {
        if(serr == "OPEN FAILED")
        {
            print("Try to open file name : %s\n \
                   At : %s failed \n            \
                   Failed\n",         
                   _full_path.filename() /= _full_path.extension(),
                   _full_path.parent_path()
            );
        }
    }
}

void TXT:: close()
{
    if(fd != nullptr)
        fclose(fd);
}
    

