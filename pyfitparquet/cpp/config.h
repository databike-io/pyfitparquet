#if !defined(CONFIG_H)
#define CONFIG_H

#include <regex>
#include <iostream>
#include <unordered_map>
#include "fit_profile.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3

#include "boost/filesystem.hpp"
using namespace boost::filesystem;

#define CONFIG Config::getInstance()


class Config
{
public:

    // Config singleton accessor
    static Config& getInstance() {
        static Config single_instance; 
        return single_instance;
    }

    // Re-populate from config
    bool reset() {
        param_server.clear();
        return populate_server();
    }

    // Param value accessor
    std::string& operator[]( const std::string& param_k ) {
        std::unordered_map<std::string, std::string>::
            const_iterator it = param_server.find(param_k);
        
        // std::cout << param_k << std::endl;
        assert (it != param_server.end());
        return param_server[param_k];
    }

    // Checks for existence of param in config
    bool exists( const std::string& param_k ) {
        std::unordered_map<std::string, std::string>::const_iterator it = param_server.find(param_k);
        if ( it == param_server.end() ) return false;
        return true;
    }

    std::string& manufacturer_name(FIT_MANUFACTURER fit_manfact_k) {
        return manfact_names[fit_manfact_k];
    }

    std::string& favero_product_name(FIT_FAVERO_PRODUCT fit_pfavero_k) {
        return pfavero_names[fit_pfavero_k];
    }

    std::string& garmin_product_name(FIT_GARMIN_PRODUCT fit_pgarmin_k) {
        return pgarmin_names[fit_pgarmin_k];
    }

    void print() {
        for (auto it : param_server) std::cout << "'"  << it.first << "' : '" << it.second << "'" << std::endl;
        for (auto it : manfact_names) std::cout << it.first << " : " << it.second << std::endl;
        for (auto it : pfavero_names) std::cout << it.first << " : " << it.second << std::endl;
        for (auto it : pgarmin_names) std::cout << it.first << " : " << it.second << std::endl;
    }

    // Explicitly remove copy ctor and assignment
    Config(Config const&)         = delete;
    void operator=(Config const&) = delete;

private:

    // Called on construction and reset
    bool populate_server() {
        path parquet_config, parquet_config_base;
        char *conda_prefix_env = std::getenv("CONDA_PREFIX");
        char *pyfit_config_env = std::getenv("PYFIT_CONFIG_DIR");

        bool found = false;
        if (pyfit_config_env) {
            parquet_config = std::string(pyfit_config_env).append("/parquet_config.yml");
            found = boost::filesystem::exists(parquet_config);
        }

        if (!found && conda_prefix_env) {
            found = _find_file(conda_prefix_env, "parquet_config.yml", parquet_config_base);
            if (found && pyfit_config_env) copy_file(parquet_config_base, parquet_config);
            else if (found) parquet_config = parquet_config_base;
        }
        
        if (!found) {
            std::cerr << "ERROR: unable to find: parquet_config.yml" << std::endl;
            return false;
        }
        
        ifstream config_fhandle(parquet_config);
        if (!config_fhandle.is_open()) {
            std::cerr << "ERROR: unable to open: " << parquet_config << std::endl; 
            return false;
        }

        _parse_config_file(config_fhandle);
        config_fhandle.close();
        return true;
    }

    // Finds file_name (if exists) anywhere at or deeper than start_dir
    bool _find_file(const path& start_dir, const path& file_name, path& path_found) {
        const recursive_directory_iterator end;
        const auto it = std::find_if(recursive_directory_iterator(start_dir), end,
            [&file_name](const directory_entry& e) { return e.path().filename() == file_name;});
        
        if (it == end) return false;
        path_found = it->path();
        return true;
    }

    // Parse config file using regex matches to load param_server
    void _parse_config_file(ifstream &config_fhandle) {
        std::string line;
        std::smatch matchobj;
        std::regex rg_comment("\\s*\\#.*");
        std::regex rg_parameter("\\s*(\\w+)\\s*:\\s*(\\w+).*");

        while (std::getline(config_fhandle, line))
        {
            // Strip comment lines
            if (std::regex_match(line, matchobj, rg_comment)) 
                continue; 

            // Match 'param : value' pairs
            if (std::regex_match(line, matchobj, rg_parameter))
                param_server.insert({matchobj.str(1), matchobj.str(2)});
        }
    }

    // Configuration hashmaps
    std::unordered_map<std::string, std::string> param_server;
    std::unordered_map<FIT_MANUFACTURER, std::string> manfact_names;
    std::unordered_map<FIT_FAVERO_PRODUCT, std::string> pfavero_names;
    std::unordered_map<FIT_GARMIN_PRODUCT, std::string> pgarmin_names;

    // Ctor private
    Config() {
        populate_server();
        popul_manfact_names();
        popul_favero_product_names();
        popul_garmin_product_names();
    }

    void popul_manfact_names() {
        manfact_names.insert({FIT_MANUFACTURER_INVALID, "INVALID"});
        manfact_names.insert({FIT_MANUFACTURER_GARMIN, "GARMIN"});
        manfact_names.insert({FIT_MANUFACTURER_GARMIN_FR405_ANTFS, "GARMIN_FR405_ANTFS"});
        manfact_names.insert({FIT_MANUFACTURER_ZEPHYR, "ZEPHYR"});
        manfact_names.insert({FIT_MANUFACTURER_DAYTON, "DAYTON"});
        manfact_names.insert({FIT_MANUFACTURER_IDT, "IDT"});
        manfact_names.insert({FIT_MANUFACTURER_SRM, "SRM"});
        manfact_names.insert({FIT_MANUFACTURER_QUARQ, "QUARQ"});
        manfact_names.insert({FIT_MANUFACTURER_IBIKE, "IBIKE"});
        manfact_names.insert({FIT_MANUFACTURER_SARIS, "SARIS"});
        manfact_names.insert({FIT_MANUFACTURER_SPARK_HK, "SPARK_HK"});
        manfact_names.insert({FIT_MANUFACTURER_TANITA, "TANITA"});
        manfact_names.insert({FIT_MANUFACTURER_ECHOWELL, "ECHOWELL"});
        manfact_names.insert({FIT_MANUFACTURER_DYNASTREAM_OEM, "DYNASTREAM_OEM"});
        manfact_names.insert({FIT_MANUFACTURER_NAUTILUS, "NAUTILUS"});
        manfact_names.insert({FIT_MANUFACTURER_DYNASTREAM, "DYNASTREAM"});
        manfact_names.insert({FIT_MANUFACTURER_TIMEX, "TIMEX"});
        manfact_names.insert({FIT_MANUFACTURER_METRIGEAR, "METRIGEAR"});
        manfact_names.insert({FIT_MANUFACTURER_XELIC, "XELIC"});
        manfact_names.insert({FIT_MANUFACTURER_BEURER, "BEURER"});
        manfact_names.insert({FIT_MANUFACTURER_CARDIOSPORT, "CARDIOSPORT"});
        manfact_names.insert({FIT_MANUFACTURER_A_AND_D, "A_AND_D"});
        manfact_names.insert({FIT_MANUFACTURER_HMM, "HMM"});
        manfact_names.insert({FIT_MANUFACTURER_SUUNTO, "SUUNTO"});
        manfact_names.insert({FIT_MANUFACTURER_THITA_ELEKTRONIK, "THITA_ELEKTRONIK"});
        manfact_names.insert({FIT_MANUFACTURER_GPULSE, "GPULSE"});
        manfact_names.insert({FIT_MANUFACTURER_CLEAN_MOBILE, "CLEAN_MOBILE"});
        manfact_names.insert({FIT_MANUFACTURER_PEDAL_BRAIN, "PEDAL_BRAIN"});
        manfact_names.insert({FIT_MANUFACTURER_PEAKSWARE, "PEAKSWARE"});
        manfact_names.insert({FIT_MANUFACTURER_SAXONAR, "SAXONAR"});
        manfact_names.insert({FIT_MANUFACTURER_LEMOND_FITNESS, "LEMOND_FITNESS"});
        manfact_names.insert({FIT_MANUFACTURER_DEXCOM, "DEXCOM"});
        manfact_names.insert({FIT_MANUFACTURER_WAHOO_FITNESS, "WAHOO_FITNESS"});
        manfact_names.insert({FIT_MANUFACTURER_OCTANE_FITNESS, "OCTANE_FITNESS"});
        manfact_names.insert({FIT_MANUFACTURER_ARCHINOETICS, "ARCHINOETICS"});
        manfact_names.insert({FIT_MANUFACTURER_THE_HURT_BOX, "THE_HURT_BOX"});
        manfact_names.insert({FIT_MANUFACTURER_CITIZEN_SYSTEMS, "CITIZEN_SYSTEMS"});
        manfact_names.insert({FIT_MANUFACTURER_MAGELLAN, "MAGELLAN"});
        manfact_names.insert({FIT_MANUFACTURER_OSYNCE, "OSYNCE"});
        manfact_names.insert({FIT_MANUFACTURER_HOLUX, "HOLUX"});
        manfact_names.insert({FIT_MANUFACTURER_CONCEPT2, "CONCEPT2"});
        manfact_names.insert({FIT_MANUFACTURER_ONE_GIANT_LEAP, "ONE_GIANT_LEAP"});
        manfact_names.insert({FIT_MANUFACTURER_ACE_SENSOR, "ACE_SENSOR"});
        manfact_names.insert({FIT_MANUFACTURER_BRIM_BROTHERS, "BRIM_BROTHERS"});
        manfact_names.insert({FIT_MANUFACTURER_XPLOVA, "XPLOVA"});
        manfact_names.insert({FIT_MANUFACTURER_PERCEPTION_DIGITAL, "PERCEPTION_DIGITAL"});
        manfact_names.insert({FIT_MANUFACTURER_BF1SYSTEMS, "BF1SYSTEMS"});
        manfact_names.insert({FIT_MANUFACTURER_PIONEER, "PIONEER"});
        manfact_names.insert({FIT_MANUFACTURER_SPANTEC, "SPANTEC"});
        manfact_names.insert({FIT_MANUFACTURER_METALOGICS, "METALOGICS"});
        manfact_names.insert({FIT_MANUFACTURER_4IIIIS, "4IIIIS"});
        manfact_names.insert({FIT_MANUFACTURER_SEIKO_EPSON, "SEIKO_EPSON"});
        manfact_names.insert({FIT_MANUFACTURER_SEIKO_EPSON_OEM, "SEIKO_EPSON_OEM"});
        manfact_names.insert({FIT_MANUFACTURER_IFOR_POWELL, "IFOR_POWELL"});
        manfact_names.insert({FIT_MANUFACTURER_MAXWELL_GUIDER, "MAXWELL_GUIDER"});
        manfact_names.insert({FIT_MANUFACTURER_STAR_TRAC, "STAR_TRAC"});
        manfact_names.insert({FIT_MANUFACTURER_BREAKAWAY, "BREAKAWAY"});
        manfact_names.insert({FIT_MANUFACTURER_ALATECH_TECHNOLOGY_LTD, "ALATECH_TECHNOLOGY_LTD"});
        manfact_names.insert({FIT_MANUFACTURER_MIO_TECHNOLOGY_EUROPE, "MIO_TECHNOLOGY_EUROPE"});
        manfact_names.insert({FIT_MANUFACTURER_ROTOR, "ROTOR"});
        manfact_names.insert({FIT_MANUFACTURER_GEONAUTE, "GEONAUTE"});
        manfact_names.insert({FIT_MANUFACTURER_ID_BIKE, "ID_BIKE"});
        manfact_names.insert({FIT_MANUFACTURER_SPECIALIZED, "SPECIALIZED"});
        manfact_names.insert({FIT_MANUFACTURER_WTEK, "WTEK"});
        manfact_names.insert({FIT_MANUFACTURER_PHYSICAL_ENTERPRISES, "PHYSICAL_ENTERPRISES"});
        manfact_names.insert({FIT_MANUFACTURER_NORTH_POLE_ENGINEERING, "NORTH_POLE_ENGINEERING"});
        manfact_names.insert({FIT_MANUFACTURER_BKOOL, "BKOOL"});
        manfact_names.insert({FIT_MANUFACTURER_CATEYE, "CATEYE"});
        manfact_names.insert({FIT_MANUFACTURER_STAGES_CYCLING, "STAGES_CYCLING"});
        manfact_names.insert({FIT_MANUFACTURER_SIGMASPORT, "SIGMASPORT"});
        manfact_names.insert({FIT_MANUFACTURER_TOMTOM, "TOMTOM"});
        manfact_names.insert({FIT_MANUFACTURER_PERIPEDAL, "PERIPEDAL"});
        manfact_names.insert({FIT_MANUFACTURER_WATTBIKE, "WATTBIKE"});
        manfact_names.insert({FIT_MANUFACTURER_MOXY, "MOXY"});
        manfact_names.insert({FIT_MANUFACTURER_CICLOSPORT, "CICLOSPORT"});
        manfact_names.insert({FIT_MANUFACTURER_POWERBAHN, "POWERBAHN"});
        manfact_names.insert({FIT_MANUFACTURER_ACORN_PROJECTS_APS, "ACORN_PROJECTS_APS"});
        manfact_names.insert({FIT_MANUFACTURER_LIFEBEAM, "LIFEBEAM"});
        manfact_names.insert({FIT_MANUFACTURER_BONTRAGER, "BONTRAGER"});
        manfact_names.insert({FIT_MANUFACTURER_WELLGO, "WELLGO"});
        manfact_names.insert({FIT_MANUFACTURER_SCOSCHE, "SCOSCHE"});
        manfact_names.insert({FIT_MANUFACTURER_MAGURA, "MAGURA"});
        manfact_names.insert({FIT_MANUFACTURER_WOODWAY, "WOODWAY"});
        manfact_names.insert({FIT_MANUFACTURER_ELITE, "ELITE"});
        manfact_names.insert({FIT_MANUFACTURER_NIELSEN_KELLERMAN, "NIELSEN_KELLERMAN"});
        manfact_names.insert({FIT_MANUFACTURER_DK_CITY, "DK_CITY"});
        manfact_names.insert({FIT_MANUFACTURER_TACX, "TACX"});
        manfact_names.insert({FIT_MANUFACTURER_DIRECTION_TECHNOLOGY, "DIRECTION_TECHNOLOGY"});
        manfact_names.insert({FIT_MANUFACTURER_MAGTONIC, "MAGTONIC"});
        manfact_names.insert({FIT_MANUFACTURER_1PARTCARBON, "1PARTCARBON"});
        manfact_names.insert({FIT_MANUFACTURER_INSIDE_RIDE_TECHNOLOGIES, "INSIDE_RIDE_TECHNOLOGIES"});
        manfact_names.insert({FIT_MANUFACTURER_SOUND_OF_MOTION, "SOUND_OF_MOTION"});
        manfact_names.insert({FIT_MANUFACTURER_STRYD, "STRYD"});
        manfact_names.insert({FIT_MANUFACTURER_ICG, "ICG"});
        manfact_names.insert({FIT_MANUFACTURER_MIPULSE, "MIPULSE"});
        manfact_names.insert({FIT_MANUFACTURER_BSX_ATHLETICS, "BSX_ATHLETICS"});
        manfact_names.insert({FIT_MANUFACTURER_LOOK, "LOOK"});
        manfact_names.insert({FIT_MANUFACTURER_CAMPAGNOLO_SRL, "CAMPAGNOLO_SRL"});
        manfact_names.insert({FIT_MANUFACTURER_BODY_BIKE_SMART, "BODY_BIKE_SMART"});
        manfact_names.insert({FIT_MANUFACTURER_PRAXISWORKS, "PRAXISWORKS"});
        manfact_names.insert({FIT_MANUFACTURER_LIMITS_TECHNOLOGY, "LIMITS_TECHNOLOGY"});
        manfact_names.insert({FIT_MANUFACTURER_TOPACTION_TECHNOLOGY, "TOPACTION_TECHNOLOGY"});
        manfact_names.insert({FIT_MANUFACTURER_COSINUSS, "COSINUSS"});
        manfact_names.insert({FIT_MANUFACTURER_FITCARE, "FITCARE"});
        manfact_names.insert({FIT_MANUFACTURER_MAGENE, "MAGENE"});
        manfact_names.insert({FIT_MANUFACTURER_GIANT_MANUFACTURING_CO, "GIANT_MANUFACTURING_CO"});
        manfact_names.insert({FIT_MANUFACTURER_TIGRASPORT, "TIGRASPORT"});
        manfact_names.insert({FIT_MANUFACTURER_SALUTRON, "SALUTRON"});
        manfact_names.insert({FIT_MANUFACTURER_TECHNOGYM, "TECHNOGYM"});
        manfact_names.insert({FIT_MANUFACTURER_BRYTON_SENSORS, "BRYTON_SENSORS"});
        manfact_names.insert({FIT_MANUFACTURER_LATITUDE_LIMITED, "LATITUDE_LIMITED"});
        manfact_names.insert({FIT_MANUFACTURER_SOARING_TECHNOLOGY, "SOARING_TECHNOLOGY"});
        manfact_names.insert({FIT_MANUFACTURER_IGPSPORT, "IGPSPORT"});
        manfact_names.insert({FIT_MANUFACTURER_THINKRIDER, "THINKRIDER"});
        manfact_names.insert({FIT_MANUFACTURER_GOPHER_SPORT, "GOPHER_SPORT"});
        manfact_names.insert({FIT_MANUFACTURER_WATERROWER, "WATERROWER"});
        manfact_names.insert({FIT_MANUFACTURER_ORANGETHEORY, "ORANGETHEORY"});
        manfact_names.insert({FIT_MANUFACTURER_INPEAK, "INPEAK"});
        manfact_names.insert({FIT_MANUFACTURER_KINETIC, "KINETIC"});
        manfact_names.insert({FIT_MANUFACTURER_JOHNSON_HEALTH_TECH, "JOHNSON_HEALTH_TECH"});
        manfact_names.insert({FIT_MANUFACTURER_POLAR_ELECTRO, "POLAR_ELECTRO"});
        manfact_names.insert({FIT_MANUFACTURER_SEESENSE, "SEESENSE"});
        manfact_names.insert({FIT_MANUFACTURER_NCI_TECHNOLOGY, "NCI_TECHNOLOGY"});
        manfact_names.insert({FIT_MANUFACTURER_IQSQUARE, "IQSQUARE"});
        manfact_names.insert({FIT_MANUFACTURER_LEOMO, "LEOMO"});
        manfact_names.insert({FIT_MANUFACTURER_IFIT_COM, "IFIT_COM"});
        manfact_names.insert({FIT_MANUFACTURER_COROS_BYTE, "COROS_BYTE"});
        manfact_names.insert({FIT_MANUFACTURER_VERSA_DESIGN, "VERSA_DESIGN"});
        manfact_names.insert({FIT_MANUFACTURER_CHILEAF, "CHILEAF"});
        manfact_names.insert({FIT_MANUFACTURER_CYCPLUS, "CYCPLUS"});
        manfact_names.insert({FIT_MANUFACTURER_GRAVAA_BYTE, "GRAVAA_BYTE"});
        manfact_names.insert({FIT_MANUFACTURER_DEVELOPMENT, "DEVELOPMENT"});
        manfact_names.insert({FIT_MANUFACTURER_HEALTHANDLIFE, "HEALTHANDLIFE"});
        manfact_names.insert({FIT_MANUFACTURER_LEZYNE, "LEZYNE"});
        manfact_names.insert({FIT_MANUFACTURER_SCRIBE_LABS, "SCRIBE_LABS"});
        manfact_names.insert({FIT_MANUFACTURER_ZWIFT, "ZWIFT"});
        manfact_names.insert({FIT_MANUFACTURER_WATTEAM, "WATTEAM"});
        manfact_names.insert({FIT_MANUFACTURER_RECON, "RECON"});
        manfact_names.insert({FIT_MANUFACTURER_FAVERO_ELECTRONICS, "FAVERO_ELECTRONICS"});
        manfact_names.insert({FIT_MANUFACTURER_DYNOVELO, "DYNOVELO"});
        manfact_names.insert({FIT_MANUFACTURER_STRAVA, "STRAVA"});
        manfact_names.insert({FIT_MANUFACTURER_PRECOR, "PRECOR"});
        manfact_names.insert({FIT_MANUFACTURER_BRYTON, "BRYTON"});
        manfact_names.insert({FIT_MANUFACTURER_SRAM, "SRAM"});
        manfact_names.insert({FIT_MANUFACTURER_NAVMAN, "NAVMAN"});
        manfact_names.insert({FIT_MANUFACTURER_COBI, "COBI"});
        manfact_names.insert({FIT_MANUFACTURER_SPIVI, "SPIVI"});
        manfact_names.insert({FIT_MANUFACTURER_MIO_MAGELLAN, "MIO_MAGELLAN"});
        manfact_names.insert({FIT_MANUFACTURER_EVESPORTS, "EVESPORTS"});
        manfact_names.insert({FIT_MANUFACTURER_SENSITIVUS_GAUGE, "SENSITIVUS_GAUGE"});
        manfact_names.insert({FIT_MANUFACTURER_PODOON, "PODOON"});
        manfact_names.insert({FIT_MANUFACTURER_LIFE_TIME_FITNESS, "LIFE_TIME_FITNESS"});
        manfact_names.insert({FIT_MANUFACTURER_FALCO_E_MOTORS, "FALCO_E_MOTORS"});
        manfact_names.insert({FIT_MANUFACTURER_MINOURA, "MINOURA"});
        manfact_names.insert({FIT_MANUFACTURER_CYCLIQ, "CYCLIQ"});
        manfact_names.insert({FIT_MANUFACTURER_LUXOTTICA, "LUXOTTICA"});
        manfact_names.insert({FIT_MANUFACTURER_TRAINER_ROAD, "TRAINER_ROAD"});
        manfact_names.insert({FIT_MANUFACTURER_THE_SUFFERFEST, "THE_SUFFERFEST"});
        manfact_names.insert({FIT_MANUFACTURER_FULLSPEEDAHEAD, "FULLSPEEDAHEAD"});
        manfact_names.insert({FIT_MANUFACTURER_VIRTUALTRAINING, "VIRTUALTRAINING"});
        manfact_names.insert({FIT_MANUFACTURER_FEEDBACKSPORTS, "FEEDBACKSPORTS"});
        manfact_names.insert({FIT_MANUFACTURER_OMATA, "OMATA"});
        manfact_names.insert({FIT_MANUFACTURER_VDO, "VDO"});
        manfact_names.insert({FIT_MANUFACTURER_MAGNETICDAYS, "MAGNETICDAYS"});
        manfact_names.insert({FIT_MANUFACTURER_HAMMERHEAD, "HAMMERHEAD"});
        manfact_names.insert({FIT_MANUFACTURER_KINETIC_BY_KURT, "KINETIC_BY_KURT"});
        manfact_names.insert({FIT_MANUFACTURER_SHAPELOG, "SHAPELOG"});
        manfact_names.insert({FIT_MANUFACTURER_DABUZIDUO, "DABUZIDUO"});
        manfact_names.insert({FIT_MANUFACTURER_JETBLACK, "JETBLACK"});
        manfact_names.insert({FIT_MANUFACTURER_COROS, "COROS"});
        manfact_names.insert({FIT_MANUFACTURER_VIRTUGO, "VIRTUGO"});
        manfact_names.insert({FIT_MANUFACTURER_VELOSENSE, "VELOSENSE"});
        manfact_names.insert({FIT_MANUFACTURER_CYCLIGENTINC, "CYCLIGENTINC"});
        manfact_names.insert({FIT_MANUFACTURER_TRAILFORKS, "TRAILFORKS"});
        manfact_names.insert({FIT_MANUFACTURER_MAHLE_EBIKEMOTION, "MAHLE_EBIKEMOTION"});
        manfact_names.insert({FIT_MANUFACTURER_NURVV, "NURVV"});
        manfact_names.insert({FIT_MANUFACTURER_MICROPROGRAM, "MICROPROGRAM"});
        manfact_names.insert({FIT_MANUFACTURER_ZONE5CLOUD, "ZONE5CLOUD"});
        manfact_names.insert({FIT_MANUFACTURER_GREENTEG, "GREENTEG"});
        manfact_names.insert({FIT_MANUFACTURER_YAMAHA_MOTORS, "YAMAHA_MOTORS"});
        manfact_names.insert({FIT_MANUFACTURER_WHOOP, "WHOOP"});
        manfact_names.insert({FIT_MANUFACTURER_GRAVAA, "GRAVAA"});
        manfact_names.insert({FIT_MANUFACTURER_ONELAP, "ONELAP"});
        manfact_names.insert({FIT_MANUFACTURER_MONARK_EXERCISE, "MONARK_EXERCISE"});
        manfact_names.insert({FIT_MANUFACTURER_FORM, "FORM"});
        manfact_names.insert({FIT_MANUFACTURER_ACTIGRAPHCORP, "ACTIGRAPHCORP"});
    }

    void popul_favero_product_names() {
        pfavero_names.insert({FIT_FAVERO_PRODUCT_INVALID, "INVALID"});
        pfavero_names.insert({FIT_FAVERO_PRODUCT_ASSIOMA_UNO, "ASSIOMA_UNO"});
        pfavero_names.insert({FIT_FAVERO_PRODUCT_ASSIOMA_DUO, "ASSIOMA_DUO"});
    }

    void popul_garmin_product_names() {
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_INVALID, "INVALID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM1, "HRM1"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_AXH01, "AXH01"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_AXB01, "AXB01"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_AXB02, "AXB02"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM2SS, "HRM2SS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_DSI_ALF02, "DSI_ALF02"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM3SS, "HRM3SS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM_RUN_SINGLE_BYTE_PRODUCT_ID, "HRM_RUN_SINGLE_BYTE_PRODUCT_ID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_BSM, "BSM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_BCM, "BCM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_AXS01, "AXS01"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM_TRI_SINGLE_BYTE_PRODUCT_ID, "HRM_TRI_SINGLE_BYTE_PRODUCT_ID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM4_RUN_SINGLE_BYTE_PRODUCT_ID, "HRM4_RUN_SINGLE_BYTE_PRODUCT_ID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR225_SINGLE_BYTE_PRODUCT_ID, "FR225_SINGLE_BYTE_PRODUCT_ID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_GEN3_BSM_SINGLE_BYTE_PRODUCT_ID, "GEN3_BSM_SINGLE_BYTE_PRODUCT_ID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_GEN3_BCM_SINGLE_BYTE_PRODUCT_ID, "GEN3_BCM_SINGLE_BYTE_PRODUCT_ID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR301_CHINA, "FR301_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR301_JAPAN, "FR301_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR301_KOREA, "FR301_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR301_TAIWAN, "FR301_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR405, "FR405"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR50, "FR50"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR405_JAPAN, "FR405_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR60, "FR60"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_DSI_ALF01, "DSI_ALF01"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR310XT, "FR310XT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE500, "EDGE500"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR110, "FR110"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE800, "EDGE800"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE500_TAIWAN, "EDGE500_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE500_JAPAN, "EDGE500_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_CHIRP, "CHIRP"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR110_JAPAN, "FR110_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE200, "EDGE200"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR910XT, "FR910XT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE800_TAIWAN, "EDGE800_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE800_JAPAN, "EDGE800_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_ALF04, "ALF04"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR610, "FR610"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR210_JAPAN, "FR210_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VECTOR_SS, "VECTOR_SS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VECTOR_CP, "VECTOR_CP"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE800_CHINA, "EDGE800_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE500_CHINA, "EDGE500_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_G10, "APPROACH_G10"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR610_JAPAN, "FR610_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE500_KOREA, "EDGE500_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR70, "FR70"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR310XT_4T, "FR310XT_4T"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_AMX, "AMX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR10, "FR10"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE800_KOREA, "EDGE800_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_SWIM, "SWIM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR910XT_CHINA, "FR910XT_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX, "FENIX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE200_TAIWAN, "EDGE200_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE510, "EDGE510"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE810, "EDGE810"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TEMPE, "TEMPE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR910XT_JAPAN, "FR910XT_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR620, "FR620"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR220, "FR220"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR910XT_KOREA, "FR910XT_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR10_JAPAN, "FR10_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE810_JAPAN, "EDGE810_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIRB_ELITE, "VIRB_ELITE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_TOURING, "EDGE_TOURING"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE510_JAPAN, "EDGE510_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM_TRI, "HRM_TRI"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM_RUN, "HRM_RUN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR920XT, "FR920XT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE510_ASIA, "EDGE510_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE810_CHINA, "EDGE810_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE810_TAIWAN, "EDGE810_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE1000, "EDGE1000"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_FIT, "VIVO_FIT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIRB_REMOTE, "VIRB_REMOTE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_KI, "VIVO_KI"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR15, "FR15"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE, "VIVO_ACTIVE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE510_KOREA, "EDGE510_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR620_JAPAN, "FR620_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR620_CHINA, "FR620_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR220_JAPAN, "FR220_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR220_CHINA, "FR220_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_S6, "APPROACH_S6"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART, "VIVO_SMART"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX2, "FENIX2"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EPIX, "EPIX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3, "FENIX3"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE1000_TAIWAN, "EDGE1000_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE1000_JAPAN, "EDGE1000_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR15_JAPAN, "FR15_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE520, "EDGE520"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE1000_CHINA, "EDGE1000_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR620_RUSSIA, "FR620_RUSSIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR220_RUSSIA, "FR220_RUSSIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VECTOR_S, "VECTOR_S"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE1000_KOREA, "EDGE1000_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR920XT_TAIWAN, "FR920XT_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR920XT_CHINA, "FR920XT_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR920XT_JAPAN, "FR920XT_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIRBX, "VIRBX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART_APAC, "VIVO_SMART_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_ETREX_TOUCH, "ETREX_TOUCH"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE25, "EDGE25"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR25, "FR25"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_FIT2, "VIVO_FIT2"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR225, "FR225"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR630, "FR630"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR230, "FR230"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR735XT, "FR735XT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE_APAC, "VIVO_ACTIVE_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VECTOR_2, "VECTOR_2"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VECTOR_2S, "VECTOR_2S"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIRBXE, "VIRBXE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR620_TAIWAN, "FR620_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR220_TAIWAN, "FR220_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TRUSWING, "TRUSWING"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_D2AIRVENU, "D2AIRVENU"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_CHINA, "FENIX3_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_TWN, "FENIX3_TWN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_HEADLIGHT, "VARIA_HEADLIGHT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_TAILLIGHT_OLD, "VARIA_TAILLIGHT_OLD"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_EXPLORE_1000, "EDGE_EXPLORE_1000"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR225_ASIA, "FR225_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_RADAR_TAILLIGHT, "VARIA_RADAR_TAILLIGHT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_RADAR_DISPLAY, "VARIA_RADAR_DISPLAY"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE20, "EDGE20"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE520_ASIA, "EDGE520_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE520_JAPAN, "EDGE520_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_D2_BRAVO, "D2_BRAVO"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_S20, "APPROACH_S20"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART2, "VIVO_SMART2"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE1000_THAI, "EDGE1000_THAI"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_REMOTE, "VARIA_REMOTE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE25_ASIA, "EDGE25_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE25_JPN, "EDGE25_JPN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE20_ASIA, "EDGE20_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_X40, "APPROACH_X40"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_JAPAN, "FENIX3_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART_EMEA, "VIVO_SMART_EMEA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR630_ASIA, "FR630_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR630_JPN, "FR630_JPN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR230_JPN, "FR230_JPN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM4_RUN, "HRM4_RUN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EPIX_JAPAN, "EPIX_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE_HR, "VIVO_ACTIVE_HR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART_GPS_HR, "VIVO_SMART_GPS_HR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART_HR, "VIVO_SMART_HR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART_HR_ASIA, "VIVO_SMART_HR_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART_GPS_HR_ASIA, "VIVO_SMART_GPS_HR_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_MOVE, "VIVO_MOVE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_TAILLIGHT, "VARIA_TAILLIGHT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR235_JAPAN, "FR235_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_VISION, "VARIA_VISION"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_FIT3, "VIVO_FIT3"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_KOREA, "FENIX3_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_SEA, "FENIX3_SEA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_HR, "FENIX3_HR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIRB_ULTRA_30, "VIRB_ULTRA_30"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_INDEX_SMART_SCALE, "INDEX_SMART_SCALE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR235, "FR235"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_CHRONOS, "FENIX3_CHRONOS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_OREGON7XX, "OREGON7XX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_RINO7XX, "RINO7XX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EPIX_KOREA, "EPIX_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_HR_CHN, "FENIX3_HR_CHN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_HR_TWN, "FENIX3_HR_TWN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_HR_JPN, "FENIX3_HR_JPN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_HR_SEA, "FENIX3_HR_SEA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_HR_KOR, "FENIX3_HR_KOR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_NAUTIX, "NAUTIX"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE_HR_APAC, "VIVO_ACTIVE_HR_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_OREGON7XX_WW, "OREGON7XX_WW"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_820, "EDGE_820"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_EXPLORE_820, "EDGE_EXPLORE_820"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR735XT_APAC, "FR735XT_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR735XT_JAPAN, "FR735XT_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5S, "FENIX5S"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_D2_BRAVO_TITANIUM, "D2_BRAVO_TITANIUM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VARIA_UT800, "VARIA_UT800"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_RUNNING_DYNAMICS_POD, "RUNNING_DYNAMICS_POD"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_820_CHINA, "EDGE_820_CHINA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_820_JAPAN, "EDGE_820_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5X, "FENIX5X"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_FIT_JR, "VIVO_FIT_JR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART3, "VIVO_SMART3"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SPORT, "VIVO_SPORT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_820_TAIWAN, "EDGE_820_TAIWAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_820_KOREA, "EDGE_820_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_820_SEA, "EDGE_820_SEA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR35_HEBREW, "FR35_HEBREW"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_S60, "APPROACH_S60"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR35_APAC, "FR35_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR35_JAPAN, "FR35_JAPAN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX3_CHRONOS_ASIA, "FENIX3_CHRONOS_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIRB_360, "VIRB_360"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR935, "FR935"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5, "FENIX5"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOACTIVE3, "VIVOACTIVE3"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR235_CHINA_NFC, "FR235_CHINA_NFC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FORETREX_601_701, "FORETREX_601_701"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_MOVE_HR, "VIVO_MOVE_HR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_1030, "EDGE_1030"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5_ASIA, "FENIX5_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5S_ASIA, "FENIX5S_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5X_ASIA, "FENIX5X_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_Z80, "APPROACH_Z80"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR35_KOREA, "FR35_KOREA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_D2CHARLIE, "D2CHARLIE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART3_APAC, "VIVO_SMART3_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SPORT_APAC, "VIVO_SPORT_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR935_ASIA, "FR935_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_DESCENT, "DESCENT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR645, "FR645"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR645M, "FR645M"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR30, "FR30"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5S_PLUS, "FENIX5S_PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_130, "EDGE_130"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_1030_ASIA, "EDGE_1030_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOSMART_4, "VIVOSMART_4"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_MOVE_HR_ASIA, "VIVO_MOVE_HR_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_X10, "APPROACH_X10"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR30_ASIA, "FR30_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOACTIVE3M_W, "VIVOACTIVE3M_W"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR645_ASIA, "FR645_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR645M_ASIA, "FR645M_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_EXPLORE, "EDGE_EXPLORE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_GPSMAP66, "GPSMAP66"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_S10, "APPROACH_S10"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOACTIVE3M_L, "VIVOACTIVE3M_L"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_G80, "APPROACH_G80"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_130_ASIA, "EDGE_130_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_1030_BONTRAGER, "EDGE_1030_BONTRAGER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5_PLUS, "FENIX5_PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5X_PLUS, "FENIX5X_PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_520_PLUS, "EDGE_520_PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR945, "FR945"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_530, "EDGE_530"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_830, "EDGE_830"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5S_PLUS_APAC, "FENIX5S_PLUS_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX5X_PLUS_APAC, "FENIX5X_PLUS_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_520_PLUS_APAC, "EDGE_520_PLUS_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR235L_ASIA, "FR235L_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR245_ASIA, "FR245_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE3M_APAC, "VIVO_ACTIVE3M_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_GEN3_BSM, "GEN3_BSM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_GEN3_BCM, "GEN3_BCM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_SMART4_ASIA, "VIVO_SMART4_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOACTIVE4_SMALL, "VIVOACTIVE4_SMALL"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOACTIVE4_LARGE, "VIVOACTIVE4_LARGE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VENU, "VENU"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_DRIVER, "MARQ_DRIVER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_AVIATOR, "MARQ_AVIATOR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_CAPTAIN, "MARQ_CAPTAIN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_COMMANDER, "MARQ_COMMANDER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_EXPEDITION, "MARQ_EXPEDITION"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_ATHLETE, "MARQ_ATHLETE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6S_SPORT, "FENIX6S_SPORT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6S, "FENIX6S"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6_SPORT, "FENIX6_SPORT"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6, "FENIX6"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6X, "FENIX6X"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_HRM_DUAL, "HRM_DUAL"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_MOVE3_PREMIUM, "VIVO_MOVE3_PREMIUM"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_APPROACH_S40, "APPROACH_S40"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR245M_ASIA, "FR245M_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_530_APAC, "EDGE_530_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_830_APAC, "EDGE_830_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_MOVE3, "VIVO_MOVE3"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE4_SMALL_ASIA, "VIVO_ACTIVE4_SMALL_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE4_LARGE_ASIA, "VIVO_ACTIVE4_LARGE_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE4_OLED_ASIA, "VIVO_ACTIVE4_OLED_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_SWIM2, "SWIM2"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_DRIVER_ASIA, "MARQ_DRIVER_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_AVIATOR_ASIA, "MARQ_AVIATOR_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_MOVE3_ASIA, "VIVO_MOVE3_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR945_ASIA, "FR945_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVO_ACTIVE3T_CHN, "VIVO_ACTIVE3T_CHN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_CAPTAIN_ASIA, "MARQ_CAPTAIN_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_COMMANDER_ASIA, "MARQ_COMMANDER_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_EXPEDITION_ASIA, "MARQ_EXPEDITION_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_ATHLETE_ASIA, "MARQ_ATHLETE_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR45_ASIA, "FR45_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VIVOACTIVE3_DAIMLER, "VIVOACTIVE3_DAIMLER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6S_SPORT_ASIA, "FENIX6S_SPORT_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6S_ASIA, "FENIX6S_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6_SPORT_ASIA, "FENIX6_SPORT_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6_ASIA, "FENIX6_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FENIX6X_ASIA, "FENIX6X_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_130_PLUS, "EDGE_130_PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_1030_PLUS, "EDGE_1030_PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR745, "FR745"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VENUSQ, "VENUSQ"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_ADVENTURER, "MARQ_ADVENTURER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_ADVENTURER_ASIA, "MARQ_ADVENTURER_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_SWIM2_APAC, "SWIM2_APAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VENU_DAIMLER_ASIA, "VENU_DAIMLER_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_GOLFER, "MARQ_GOLFER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VENU_DAIMLER, "VENU_DAIMLER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_FR745_ASIA, "FR745_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_1030_PLUS_ASIA, "EDGE_1030_PLUS_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_130_PLUS_ASIA, "EDGE_130_PLUS_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VENUSQ_ASIA, "VENUSQ_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_MARQ_GOLFER_ASIA, "MARQ_GOLFER_ASIA"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_VENU2PLUS, "VENU2PLUS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_SDM4, "SDM4"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_EDGE_REMOTE, "EDGE_REMOTE"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TACX_TRAINING_APP_WIN, "TACX_TRAINING_APP_WIN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TACX_TRAINING_APP_MAC, "TACX_TRAINING_APP_MAC"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TRAINING_CENTER, "TRAINING_CENTER"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TACX_TRAINING_APP_ANDROID, "TACX_TRAINING_APP_ANDROID"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TACX_TRAINING_APP_IOS, "TACX_TRAINING_APP_IOS"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_TACX_TRAINING_APP_LEGACY, "TACX_TRAINING_APP_LEGACY"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_CONNECTIQ_SIMULATOR, "CONNECTIQ_SIMULATOR"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_ANDROID_ANTPLUS_PLUGIN, "ANDROID_ANTPLUS_PLUGIN"});
        pgarmin_names.insert({FIT_GARMIN_PRODUCT_CONNECT, "CONNECT"});
    }
};

#endif // defined(CONFIG_H)
