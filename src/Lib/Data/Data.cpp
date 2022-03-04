#include <Data/Data.h>

using namespace LRA_Data;
using namespace LRA_PI_Util;

/*TXT region*/
TXT::TXT(string& path, string& file_name, const char* write_mode, bool automode)
{
    // path valid 
    try
    {
        fs::path tmp = formPath(path, file_name);

        // assign parameters
        _filename = file_name;
        _path = path;
        _automode =  automode;
        strcpy(_write_mode,write_mode);
        _full_path = tmp;

        // open
        open();

    }catch(invalid_argument& e)
    {
        cerr << e.what() << endl;

    }catch(runtime_error& e)
    {
        fs::path p, f;
        std::tie(p,f) = parse2_path_name(e.what());
        cerr << "path : " << p << endl << "file : " << f << "open failed " << endl;
    }


}

TXT::~TXT()
{
    close();
}

void TXT::updatePath(string& p)
{
    if(p == _path)
    {
        print("input path : %s is same \n", p);
        return;
    }
    
    try
    {
        fs::path tmppath = formPath(p, _filename);
        if(tmppath.empty())
        {
            // create directories failed
            cerr << "path remains : " << _full_path << endl; 
            return;
        }

        //update information
        _path = p;
        _full_path = tmppath;

        // XXX:fd update
        if(_automode)
        {
            //TODO:need a mutex to lock other write behavior
            close();
            open();
        }


    }catch(invalid_argument& e)
    {
        cerr << e.what() << endl;
        cerr << "path remains : " << _full_path << endl;
    }
}

void TXT::updateFileName(string& f)
{
    if(f == _filename)
    {
        print("input filename : %s is same \n", f);
        return;
    }

    try
    {
        fs::path tmppath = formPath(_path, f);
        if(tmppath.empty())
        {
            // create directories failed
            cerr << "path remains : " << _full_path << endl; 
            return;
        }
        //update information
        _filename = f;
        _full_path = tmppath;

        // XXX:fd update 
        if(_automode)
        {
            //TODO:need a mutex to lock other write behavior
            close();
            open();
        }
    }
    catch(invalid_argument& e)
    {
        cerr << "path remains : " << _full_path << endl;
        cerr << e.what() << endl;
    }
    
}

fs::path TXT::formPath(string& path, string& filename)
{
    fs::path p{path};
    if(!fs::is_directory(p))
    {
        if(!_automode)  
            throw std::invalid_argument("path is not a directory");

        // create the directory if _automode is on
        std::error_code ec;
        if(!fs::create_directories(p, ec))
        {
            cerr << endl; 
            cerr << ec.message() << endl;
            cerr << "try to create dir : " << p << " failed " << endl;
            return "";
        }
        
    }

    p /= filename;
    p.replace_extension(".txt");

    if(fs::exists(p))
        print("File already existed, mod the file at : %s\n",p.string());
    else
        print("Create txt file at : %s\n", p.string());

    return p;
}

void TXT::open(fs::path& fullpath, const char* wmode)
{
    
    if(fd != nullptr) // already opened
        print("Already opened a txt file, please use close() first\n");
    
    
    try // try to open file
    {
        fd = fopen(fullpath.c_str(), wmode);
        if(fd == nullptr)
            throw std::runtime_error(fullpath.c_str());
        
        print("Open txt file at : %s sucessed\n", fullpath.string());
        _opened_path = fullpath;

    }catch(std::runtime_error& e)
    {
        throw;
    }
}

void TXT::open()
{
    open(_full_path, _write_mode);
}

std::tuple<fs::path, fs::path> parse2_path_name(string& s)
{
    fs::path p{s};
    return make_tuple(p.filename() /= p.extension(), p.parent_path());
}

void TXT:: close()
{
    if(fd != nullptr)
        fclose(fd);

}
    

