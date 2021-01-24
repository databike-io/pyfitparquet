#if !defined(FPTRANSFORMER_H)
#define FPTRANSFORMER_H

#include "fit.hpp"
#include "fit_mesg_listener.hpp"

#include <arrow/api.h>
#define ROW_GROUP_SIZE 20000

typedef std::shared_ptr<arrow::ArrayBuilder> pBuilder;
enum FIELD_TYPE { INT_VALUE, FLOAT_VALUE, STRING_VALUE };

class FPTransformer : public fit::MesgListener
{
public:

    FPTransformer();

    // The public FIT => Parquet function (resets transformer on completion)
    int fit_to_parquet(const char fit_fname[], const char parquet_fname[]);

    // Re-parse configuration file
    void reset_from_config();

    // MesgListener callback override,
    // meant for fit::MesgBroadcasters only
    void OnMesg(fit::Mesg& mesg) override;

private:

    // Source file name/uri (type is always: FIT)
    std::string source_filename;
    std::string source_file_uri;

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

    // Generates schema based on parquet_config.yml
    std::shared_ptr<arrow::Schema> _get_schema();

    // Internally used helper fncs
    void _init_from_config(std::unordered_map<std::string, bool> &cflags,
                           std::unordered_map<std::string, bool> &exflags,
                           std::unordered_map<std::string, pBuilder> &cbuilders);

    std::tuple<FIELD_TYPE, FIT_SINT64, FIT_FLOAT64> _get_field_type(
        const fit::FieldBase& field, const std::string &sval, FIT_UINT8 j);

    void _append_mesg_fields(fit::Mesg& mesg);
    void _append_field_fields(const fit::FieldBase& field, const std::string &sval, FIT_UINT8 j);
    void _write_parquet(const char parquet_fname[]);
    void _reset_state();
};

#endif // defined(FPTRANSFORMER_H)
