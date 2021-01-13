#if !defined(FPTRANSFORMER_H)
#define FPTRANSFORMER_H

#include "fit.hpp"
#include "fit_mesg_listener.hpp"

#include <arrow/api.h>
#define ROW_GROUP_SIZE 20000

typedef std::shared_ptr<arrow::ArrayBuilder> pBuilder;

class FPTransformer : public fit::MesgListener
{
public:

    // The public FIT => Parquet tranformation function uses fresh  
    // FPTransformer object init'd from parquet_config.yml each call
    static int fit_to_parquet(const char fit_fname[], const char parquet_fname[]);
    
    // Callback for fit::MesgBroadcaster
    void OnMesg(fit::Mesg& mesg) override;

    // Explicitly remove copy ctor and assignment
    FPTransformer(FPTransformer const&)  = delete;
    void operator=(FPTransformer const&) = delete;

private:

    // Fit file name/uri
    std::string fit_filename;
    std::string fit_file_uri;

    // Fit file "primary-ish" key
    FIT_DATE_TIME time_created;
    FIT_MANUFACTURER manufacturer_index;
    std::string manufacturer_name;
    FIT_UINT16 product_index;
    std::string product_name;

    // Arrow table config/staging objects
    std::vector<std::string> colkeys;
    std::unordered_map<std::string, bool> colflags;
    std::unordered_map<std::string, bool> excludeflags;
    std::unordered_map<std::string, pBuilder> builders;

    // Private ctor
    FPTransformer();

    // Generates schema based on parquet_config.yml
    std::shared_ptr<arrow::Schema> _get_schema();

    // Internally used helper fncs 
    int _transform(const char fit_fname[], const char parquet_fname[]);
    void _init_from_config(std::unordered_map<std::string, bool> &cflags,
                           std::unordered_map<std::string, bool> &exflags,
                           std::unordered_map<std::string, pBuilder> &cbuilders);

    void _append_mesg_fields(fit::Mesg& mesg);
    void _append_field_fields(const fit::FieldBase& field, const std::string &sval, FIT_UINT8 j);
    void _append_float(FIT_FLOAT64 fval);
    void _append_integer(FIT_SINT64 ival);
    void _append_string(const std::string &sval);
    void _write_parquet(const char parquet_fname[]);
};

#endif // defined(FPTRANSFORMER_H)
