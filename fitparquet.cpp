#include <fstream>
#include <iostream>
#include <chrono>

#include "fit_decode.hpp"
#include "fit_mesg_broadcaster.hpp"
#include "fit_developer_field_description.hpp"
#include "fit_unicode.hpp"

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>
#include <arrow/util/logging.h>

#define ROW_GROUP_SIZE 20000

class ParquetRow {
public:

    ParquetRow(FIT_MANUFACTURER manufacturer_a, FIT_UINT16 product_a, 
        FIT_UINT16 mesg_index_a, std::string mesg_name_a) 
        : manufacturer((std::int32_t)manufacturer_a), product((std::int32_t)product_a), 
          mesg_index((std::int32_t)mesg_index_a), mesg_name(mesg_name_a), field_type("string") {}

    void set_timestamp(FIT_DATE_TIME timestamp_a)     { timestamp = (std::int64_t)timestamp_a; }
    void set_field_type(std::string field_type_a)     { field_type = field_type_a; }
    void set_field_name(std::string field_name_a)     { field_name = field_name_a; }
    void set_value_string(std::string value_string_a) { value_string = value_string_a; }
    void set_units(std::string units_a)               { units = units_a; }

    void set_value_numeric(FIT_FLOAT64 value_numeric_a) { 
        value_numeric = (double)value_numeric_a;
        set_value_string(std::to_string(value_numeric_a));
    }

    // Note: use float64 values for all numeric types 
    // to assure scale and offset properly applied.
    void extract_field(const fit::FieldBase& field, FIT_UINT8 j)
    {
        set_field_name(field.GetName());
        set_units(field.GetUnits());
        switch (field.GetType())
        {
        case FIT_BASE_TYPE_ENUM:
        case FIT_BASE_TYPE_BYTE:
        case FIT_BASE_TYPE_SINT8:
        case FIT_BASE_TYPE_UINT8:
        case FIT_BASE_TYPE_SINT16:
        case FIT_BASE_TYPE_UINT16:
        case FIT_BASE_TYPE_SINT32:
        case FIT_BASE_TYPE_UINT32:
        case FIT_BASE_TYPE_SINT64:
        case FIT_BASE_TYPE_UINT64:
        case FIT_BASE_TYPE_UINT8Z:
        case FIT_BASE_TYPE_UINT16Z:
        case FIT_BASE_TYPE_UINT32Z:
        case FIT_BASE_TYPE_UINT64Z:
        case FIT_BASE_TYPE_FLOAT32:
        case FIT_BASE_TYPE_FLOAT64:
            set_field_type("numeric");
            set_value_numeric(field.GetFLOAT64Value(j));
            break;
        case FIT_BASE_TYPE_STRING:
            set_field_type("string");
            set_value_string(fit::Unicode::Copy_UTF8ToStd(
                fit::Unicode::Encode_BaseToUTF8(field.GetSTRINGValue(j))));
            break;
        default: break;
        }
    }

    void print_parquet_row() {
        std::cout << "manufacturer: " << manufacturer << std::endl;
        std::cout << "product: " << product << std::endl;
        std::cout << "timestamp: " << timestamp << std::endl;
        std::cout << "mesg_index: " << mesg_index << std::endl;
        std::cout << "mesg_name: " << mesg_name << std::endl;
        std::cout << "field_type: " << field_type << std::endl;
        std::cout << "field_name: " << field_name << std::endl;
        std::cout << "value_string: " << value_string << std::endl;
        std::cout << "value_numeric: " << value_numeric << std::endl;
        std::cout << "units: " << units << std::endl << std::endl; 
    }

    // File level
    std::int32_t manufacturer;
    std::int32_t product;

    // Mesg level
    std::int64_t timestamp;
    std::int32_t mesg_index;
    std::string mesg_name;

    // Field level
    std::string field_type;
    std::string field_name;
    std::string value_string;
    double value_numeric;
    std::string units;
};

class ParquetSerializer : public fit::MesgListener
{
public:

    static std::shared_ptr<arrow::Schema> get_schema() 
    {
        static std::shared_ptr<arrow::Schema> p_ourschema;
        if (!p_ourschema) p_ourschema = arrow::schema(
            {arrow::field("manufacturer", arrow::int32(), false), 
             arrow::field("product", arrow::int32(), false), 
             arrow::field("timestamp", arrow::int64(), false), 
             arrow::field("mesg_index", arrow::int32(), false), 
             arrow::field("mesg_name", arrow::utf8(), false), 
             arrow::field("field_type", arrow::utf8(), false), 
             arrow::field("field_name", arrow::utf8(), false), 
             arrow::field("value_string", arrow::utf8(), false), 
             arrow::field("value_numeric", arrow::float64(), true), 
             arrow::field("units", arrow::utf8(), true)});
             // Note: true == optional/nullible, false == required

        return p_ourschema;
    }

    ParquetSerializer() : 
        time_created(FIT_DATE_TIME_INVALID),
        manufacturer(FIT_MANUFACTURER_INVALID),
        product(FIT_UINT16_INVALID) {}

    int fit_to_parquet(const char fit_filename[], const char parquet_filename[]) 
    {
        try 
        {
            // Open FIT file
            std::fstream fit_fhandle;
            fit_fhandle.open(fit_filename, std::ios::in | std::ios::binary);
            if (!fit_fhandle.is_open()) throw std::runtime_error(
                std::string("ERROR opening FIT file: ") + fit_filename);

            // Validate FIT file
            fit::Decode fit_decoder;
            if (!fit_decoder.CheckIntegrity(fit_fhandle)) throw std::runtime_error(
                std::string("Integrity FAILURE for FIT file: ") + fit_filename);

            // Add FIT message listener (for OnMesg)
            fit::MesgBroadcaster msg_broadcaster;
            msg_broadcaster.AddListener((fit::MesgListener &)*this);

            // Execute FIT-to-parquet serialization 
            fit_decoder.Read(fit_fhandle, msg_broadcaster);
            _write_parquet(parquet_filename);
            return 0;
        }
        catch (const std::exception& e) { std::cerr << e.what() << std::endl; }
        return 1;
    }

    void OnMesg(fit::Mesg& mesg) override
    {
        switch (mesg.GetNum())
        {
            case FIT_MESG_NUM_INVALID:  
                break; // Drops messages of type name: unknown

            case FIT_MESG_NUM_FILE_ID:
            {
                fit::FileIdMesg& fit_mesg = static_cast<fit::FileIdMesg&>(mesg); // Downcast from non-virtual base
                if (fit_mesg.IsTimeCreatedValid() == FIT_TRUE) time_created = fit_mesg.GetTimeCreated();
                if (fit_mesg.IsManufacturerValid() == FIT_TRUE) manufacturer = fit_mesg.GetManufacturer();
                if (fit_mesg.IsProductValid() == FIT_TRUE) product = fit_mesg.GetProduct();
            } // Fall thru to default

            default:
            {
                if (manufacturer != FIT_MANUFACTURER_INVALID && product != FIT_UINT16_INVALID)
                {
                    int block_i = pqrows.size();
                    FIT_DATE_TIME timestamp_a = time_created;

                    // Generate field rows
                    for (int i = 0; i < mesg.GetNumFields(); ++i) {      
                        fit::Field* field = mesg.GetFieldByIndex(i);
                        for (FIT_UINT8 j = 0; j < field->GetNumValues(); ++j) {
                            ParquetRow pr(manufacturer, product, mesg.GetNum(), mesg.GetName());
                            if (field->GetName() == "timestamp") timestamp_a = field->GetUINT32Value(j);
                            else { pr.extract_field(*field, j); pqrows.push_back(pr); }
                        }
                    }

                    // Generate dev field rows
                    for (auto dev_field : mesg.GetDeveloperFields()) {
                        for (FIT_UINT8 j = 0; j < dev_field.GetNumValues(); ++j) {
                            ParquetRow pr(manufacturer, product, mesg.GetNum(), mesg.GetName());
                            pr.extract_field(dev_field, j);
                            pqrows.push_back(pr);
                        }
                    }

                    // Finalize the timestamp on this block of rows
                    // TODO: add epoch offset of 631065600 sec
                    for (int i = block_i; i < pqrows.size(); ++i)
                        pqrows[i].set_timestamp(timestamp_a);
                }
                else std::cerr << "  Manufacturer/Product invalid, dropping: " << mesg.GetName() << std::endl;
            }
        }
    }

private:
   
    void _write_parquet(const char parquet_filename[]) 
    {
        arrow::Int32Builder manufacturer_builder;
        arrow::Int32Builder product_builder;
        arrow::Int64Builder timestamp_builder;
        arrow::Int32Builder mesg_index_builder;
        arrow::StringBuilder mesg_name_builder;
        arrow::StringBuilder field_type_builder;
        arrow::StringBuilder field_name_builder;
        arrow::StringBuilder value_string_builder;
        arrow::DoubleBuilder value_numeric_builder;
        arrow::StringBuilder units_builder;

        for (int i = 0; i < pqrows.size(); i++) {
            PARQUET_THROW_NOT_OK(manufacturer_builder.Append(pqrows[i].manufacturer));
            PARQUET_THROW_NOT_OK(product_builder.Append(pqrows[i].product));
            PARQUET_THROW_NOT_OK(timestamp_builder.Append(pqrows[i].timestamp));
            PARQUET_THROW_NOT_OK(mesg_index_builder.Append(pqrows[i].mesg_index));
            PARQUET_THROW_NOT_OK(mesg_name_builder.Append(pqrows[i].mesg_name));
            PARQUET_THROW_NOT_OK(field_type_builder.Append(pqrows[i].field_type));
            PARQUET_THROW_NOT_OK(field_name_builder.Append(pqrows[i].field_name));
            PARQUET_THROW_NOT_OK(value_string_builder.Append(pqrows[i].value_string.c_str()));

            if (pqrows[i].field_type != "numeric") { PARQUET_THROW_NOT_OK(value_numeric_builder.AppendNull()); }
            else { PARQUET_THROW_NOT_OK(value_numeric_builder.Append(pqrows[i].value_numeric)); }

            if (pqrows[i].units.length() == 0) { PARQUET_THROW_NOT_OK(units_builder.AppendNull()); }
            else { PARQUET_THROW_NOT_OK(units_builder.Append(pqrows[i].units)); }
        }

        std::shared_ptr<arrow::Array> manufacturer_array, product_array, timestamp_array, 
            mesg_index_array, mesg_name_array, field_type_array, field_name_array, 
            value_string_array, value_numeric_array, units_array;

        PARQUET_THROW_NOT_OK(manufacturer_builder.Finish(&manufacturer_array));
        PARQUET_THROW_NOT_OK(product_builder.Finish(&product_array));
        PARQUET_THROW_NOT_OK(timestamp_builder.Finish(&timestamp_array));
        PARQUET_THROW_NOT_OK(mesg_index_builder.Finish(&mesg_index_array));
        PARQUET_THROW_NOT_OK(mesg_name_builder.Finish(&mesg_name_array));
        PARQUET_THROW_NOT_OK(field_type_builder.Finish(&field_type_array));
        PARQUET_THROW_NOT_OK(field_name_builder.Finish(&field_name_array));
        PARQUET_THROW_NOT_OK(value_string_builder.Finish(&value_string_array));
        PARQUET_THROW_NOT_OK(value_numeric_builder.Finish(&value_numeric_array));
        PARQUET_THROW_NOT_OK(units_builder.Finish(&units_array));

        std::shared_ptr<arrow::Table> atable_ptr = arrow::Table::Make(ParquetSerializer::get_schema(), 
            {manufacturer_array, product_array, timestamp_array, mesg_index_array, mesg_name_array, 
             field_type_array, field_name_array, value_string_array, value_numeric_array, units_array});

        // Open and write table to parquet output file
        std::shared_ptr<::arrow::io::FileOutputStream> parquet_fhandle;
        PARQUET_ASSIGN_OR_THROW(parquet_fhandle, ::arrow::io::FileOutputStream::Open(parquet_filename));
        PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*atable_ptr, arrow::default_memory_pool(), 
                                                        parquet_fhandle, ROW_GROUP_SIZE));
    }

    // Fit file "primary-ish" key
    FIT_DATE_TIME time_created;
    FIT_MANUFACTURER manufacturer;
    FIT_UINT16 product;

    // Arrow table staging rows/objects
    std::vector<ParquetRow> pqrows;
};

int main(int argc, char* argv[])
{
   int retstatus = 1;
   if (argc == 3) {
        ParquetSerializer pserializer;
        auto tstart = std::chrono::system_clock::now();
        retstatus = pserializer.fit_to_parquet(argv[1], argv[2]);
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-tstart;
        if (retstatus == 0) std::cout << "Serialization completed in " 
            << elapsed_seconds.count() << "sec" << std::endl;
   }
   else std::cerr << "Usage: serializer <fitfile> <parquetfile>" << std::endl;
   return retstatus;
}
