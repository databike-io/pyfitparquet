#include <algorithm>
#include <iostream>
#include <regex>
#include <unordered_map>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3

#include "boost/filesystem.hpp"
using namespace boost::filesystem;

#define CONFIG_FILE_NAME "parquet_config.yml"
#define REPO_ROOT_DIRECTORY "fit-ingest"

#define CONFIG Config::getInstance()


class Config
{
public:

    // Config singleton accessor
    static Config& getInstance() {
        static Config single_instance; 
        return single_instance;
    }

    // Param value accessor
    std::string& operator[]( const std::string& param_k ) {
        return param_server[param_k];
    }

    void print() {
        for (auto it : param_server) 
            std::cout << "'"  << it.first << "' : '" 
                      << it.second << "'" << std::endl;
    }

    // Explicitly remove copy ctor and assignment
    Config(Config const&)         = delete;
    void operator=(Config const&) = delete;

private:

    // Param server/dictionary
    std::unordered_map<std::string, std::string> param_server;

    // Called on construction
    bool parse_config_file() {
        const path config_file = CONFIG_FILE_NAME;
        const path fit_ingest_root = REPO_ROOT_DIRECTORY;

        path start_dir = ".";
        path config_uri, stop_path;
        bool found = false, stop = false;
        while (!found && !stop) {
            start_dir /= "..";
            found = _find_file(start_dir, config_file, config_uri);
            stop = _find_file(start_dir, fit_ingest_root, stop_path);
        }

        if (!found) {
            std::cerr << "ERROR: unable to find " << config_file << std::endl;
            return false;
        }

        ifstream config_fhandle(config_uri);
        if (!config_fhandle.is_open()) {
            std::cerr << "ERROR: unable to open " << config_uri << std::endl; 
            return false;
        }

        _populate_server(config_fhandle);
        config_fhandle.close();
        return true;
    }

    // Finds CONFIG_FILE_NAME (if exists) anywhere within REPO_ROOT_DIRECTORY
    bool _find_file(const path& start_dir, const path& file_name, path& path_found) {
        const recursive_directory_iterator end;
        const auto it = std::find_if(recursive_directory_iterator(start_dir), end,
            [&file_name](const directory_entry& e) { return e.path().filename() == file_name;});
        
        if (it == end) return false;
        path_found = it->path();
        return true;
    }

    // Use config file regex matches to load param_server
    void _populate_server(ifstream &config_fhandle)
    {
        std::string line;
        std::smatch matchobj;
        while (std::getline(config_fhandle, line))
        {
            std::regex rg_comment("\\s*\\#.*");
            if (std::regex_match(line, matchobj, rg_comment)) 
                continue; // Strip comment lines

            std::regex rg_parameter("\\s*(\\w+)\\s*:\\s*(\\w+).*");
            if (std::regex_match(line, matchobj, rg_parameter))
                param_server.insert({matchobj.str(1), matchobj.str(2)});
        }
    }

    // Ctor private
    Config() {
        parse_config_file();
    }
};

int main() {
    std::cout << "epoch_format : " << CONFIG["epoch_format"] << std::endl;
    std::cout << "fit_filename : " << CONFIG["fit_filename"] << std::endl;
    std::cout << "fit_filepath : " << CONFIG["fit_filepath"] << std::endl;
    std::cout << "manufacturer_index : " << CONFIG["manufacturer_index"] << std::endl;
    std::cout << "manufacturer_name : " << CONFIG["manufacturer_name"] << std::endl;
    std::cout << "product_index : " << CONFIG["product_index"] << std::endl;
    std::cout << "product_name : " << CONFIG["product_name"] << std::endl;
    std::cout << "mesg_index : " << CONFIG["mesg_index"] << std::endl;
    std::cout << "mesg_name : " << CONFIG["mesg_name"] << std::endl;
    std::cout << "field_index : " << CONFIG["field_index"] << std::endl;
    std::cout << "field_name : " << CONFIG["field_name"] << std::endl;
    std::cout << "string_data : " << CONFIG["string_data"] << std::endl;
    std::cout << "float_data : " << CONFIG["float_data"] << std::endl;
    std::cout << "integer_data : " << CONFIG["integer_data"] << std::endl;
    std::cout << "exclude_empty_values : " << CONFIG["exclude_empty_values"] << std::endl;
    std::cout << "exclude_timestamp_values : " << CONFIG["exclude_timestamp_values"] << std::endl;
    return 0;
}
 