#include <math.h> 
#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/writer.h>

#include "fit_unicode.hpp"
#include "fit_mesg_broadcaster.hpp"

#include "fittransformer.h"
#include "config.h"

#if defined PYBIND11_PRINT_PYSTDOUT
#include <pybind11/pybind11.h>
#endif 


FitTransformer::FitTransformer() : 
    time_created(FIT_DATE_TIME_INVALID), manufacturer_index(FIT_MANUFACTURER_INVALID),
    product_index(FIT_UINT16_INVALID), colkeys{"source_filetype", "source_filename", 
    "source_file_uri", "manufacturer_index", "manufacturer_name", "product_index", 
    "product_name", "timestamp", "mesg_index", "mesg_name", "field_index", "field_name", 
    "field_type", "value_string", "value_integer", "value_float", "units"} { }

int FitTransformer::fit_to_parquet(const char fit_fname[], const char parquet_fname[]) 
{
    int status = 1;

    try {
        // Open FIT file
        std::fstream fit_fhandle;
        fit_fhandle.open(fit_fname, std::ios::in | std::ios::binary);
        if (!fit_fhandle.is_open()) throw std::runtime_error(
            std::string("ERROR opening FIT file: ") + fit_fname);

        // Validate FIT file
        fit::Decode fit_decoder;
        if (!fit_decoder.CheckIntegrity(fit_fhandle)) throw std::runtime_error(
            std::string("FIT file integrity FAILURE: ") + fit_fname);
        
        // Record FIT filename/uri
        boost::filesystem::path pfit(fit_fname);
        source_filename = pfit.filename().string();
        source_file_uri = boost::filesystem::canonical(pfit).string();

        // Finish process initialization
        fit::MesgBroadcaster msg_broadcaster;
        msg_broadcaster.AddListener((fit::MesgListener &)*this);
        _init_from_config(colflags, excludeflags, builders);

        // Execute FIT-to-parquet serialization 
        fit_decoder.Read(fit_fhandle, msg_broadcaster);
        _write_parquet(parquet_fname);
        status = 0;
    }
    #if defined PYBIND11_PRINT_PYSTDOUT
    catch (const std::exception& e) { pybind11::print(e.what()); }
    #else
    catch (const std::exception& e) { std::cerr << e.what() << std::endl; }
    #endif 

    _reset_state();
    return status;
}

void FitTransformer::reset_from_config() {
    CONFIG.reset();
    colflags.clear(); excludeflags.clear(); builders.clear();
    _init_from_config(colflags, excludeflags, builders);
}

void FitTransformer::OnMesg(fit::Mesg& mesg)
{
    switch (mesg.GetNum()) {
        case FIT_MESG_NUM_INVALID:  
            break; // Drops messages of type name: unknown

        case FIT_MESG_NUM_FILE_ID:
        {
            // Downcast from non-virtual base
            fit::FileIdMesg& fit_mesg = static_cast<fit::FileIdMesg&>(mesg);
            if (fit_mesg.IsTimeCreatedValid() == FIT_TRUE)
                time_created = fit_mesg.GetTimeCreated();

            if (fit_mesg.IsManufacturerValid() == FIT_TRUE) {
                manufacturer_index = fit_mesg.GetManufacturer();
                manufacturer_name = CONFIG.manufacturer_name(manufacturer_index);
            }
            if (fit_mesg.IsFaveroProductValid() == FIT_TRUE) {
                product_index = fit_mesg.GetFaveroProduct();
                product_name = CONFIG.favero_product_name(product_index);
            }
            else if (fit_mesg.IsGarminProductValid() == FIT_TRUE) {
                product_index = fit_mesg.GetGarminProduct();
                product_name = CONFIG.garmin_product_name(product_index);
            }
            else if (fit_mesg.IsProductValid() == FIT_TRUE) 
                product_index = fit_mesg.GetProduct();
        } // Fall thru to default

        default:
        {
            if (product_index != FIT_UINT16_INVALID &&
                manufacturer_index != FIT_MANUFACTURER_INVALID)
            {
                FIT_DATE_TIME timestamp_a = time_created;
                int nfields = 0;

                // Generate field rows
                for (int i = 0; i < mesg.GetNumFields(); ++i) {
                    fit::Field* field = mesg.GetFieldByIndex(i);
                    bool is_tstamp = (field->GetName() == "timestamp");
                    for (FIT_UINT8 j = 0; j < field->GetNumValues(); ++j) {
                        std::string sval = fit::Unicode::Copy_UTF8ToStd(
                        fit::Unicode::Encode_BaseToUTF8(field->GetSTRINGValue(j)));
                        if (excludeflags["exclude_empty_values"] && sval.length() == 0) continue;
                        else if (is_tstamp) {
                            timestamp_a = field->GetUINT32Value(j);
                            if (excludeflags["exclude_timestamp_values"])
                                continue;
                        }

                        _append_mesg_fields(mesg);                            
                        _append_field_fields(*field, sval, j);
                        nfields += 1;
                    }
                }

                // Generate dev field rows
                for (auto dev_field : mesg.GetDeveloperFields()) {
                    for (FIT_UINT8 j = 0; j < dev_field.GetNumValues(); ++j) {
                        std::string sval = fit::Unicode::Copy_UTF8ToStd(
                        fit::Unicode::Encode_BaseToUTF8(dev_field.GetSTRINGValue(j)));
                        if (excludeflags["exclude_empty_values"] && sval.length() == 0) continue;

                        _append_mesg_fields(mesg);
                        _append_field_fields(dev_field, sval, j);
                        nfields += 1;
                    }
                }

                if (colflags["timestamp"]) {
                    // Finalize timestamp on mesg block of rows
                    if (timestamp_a == FIT_DATE_TIME_INVALID)
                        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::TimestampBuilder>(
                        builders["timestamp"])->AppendNulls(nfields));
                    else if (CONFIG["epoch_format"] == "UNIX")
                        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::TimestampBuilder>(
                        builders["timestamp"])->AppendValues(std::vector<std::int64_t>(
                        nfields, static_cast<std::int64_t>(timestamp_a) + 631065600)));
                    else PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::TimestampBuilder>(
                        builders["timestamp"])->AppendValues(std::vector<std::int64_t>(
                        nfields, static_cast<std::int64_t>(timestamp_a))));
                }
            }
            #if defined PYBIND11_PRINT_PYSTDOUT
            else pybind11::print("  Manufacturer/Product invalid, dropping:", mesg.GetName());
            #else
            else std::cerr << "  Manufacturer/Product invalid, dropping: " << mesg.GetName() << std::endl;
            #endif 
        }
    }
}

void FitTransformer::_append_mesg_fields(fit::Mesg& mesg) 
{
    if (colflags["source_filetype"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["source_filetype"])->Append(std::string("FIT")));

    if (colflags["source_filename"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["source_filename"])->Append(source_filename));

    if (colflags["source_file_uri"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["source_file_uri"])->Append(source_file_uri));

    if (colflags["manufacturer_index"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int32Builder>(
            builders["manufacturer_index"])->Append(manufacturer_index));

    if (colflags["manufacturer_name"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["manufacturer_name"])->Append(manufacturer_name)); 
    
    if (colflags["product_index"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int32Builder>(
            builders["product_index"])->Append(product_index));
    
    if (colflags["product_name"]) {
        if (product_name.length() == 0)
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["product_name"])->AppendNull());
        else 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["product_name"])->Append(product_name));
    }

    if (colflags["mesg_index"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int32Builder>(
            builders["mesg_index"])->Append(mesg.GetNum()));
    
    if (colflags["mesg_name"])
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["mesg_name"])->Append(mesg.GetName()));
}

void FitTransformer::_append_field_fields(const fit::FieldBase& field, const std::string &sval, FIT_UINT8 j)
{
    if (colflags["field_index"]) {
        FIT_UINT16 field_index = field.GetNum();
        if (field_index == FIT_FIELD_NUM_INVALID)
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int32Builder>(
                builders["field_index"])->AppendNull());
        else PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int32Builder>(
            builders["field_index"])->Append(field_index));
    }

    if (colflags["field_name"]) 
        PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["field_name"])->Append(field.GetName()));

    if (colflags["units"]) {
        std::string sunit = field.GetUnits();
        if (sunit.length() == 0) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["units"])->AppendNull());
        else PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
            builders["units"])->Append(sunit));
    }

    FIELD_TYPE ftype; FIT_SINT64 ival; FIT_FLOAT64 fval;
    std::tie(ftype, ival, fval) = _get_field_type(field, sval, j);

    switch (ftype) {
    case FIELD_TYPE::INT_VALUE:
        if (colflags["field_type"]) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["field_type"])->Append("integer"));

        if (colflags["value_integer"])
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int64Builder>(
                builders["value_integer"])->Append(ival));

        if (colflags["value_float"])
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::DoubleBuilder>(
                builders["value_float"])->Append(fval));

        if (colflags["value_string"]) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["value_string"])->Append(sval));
        break;

    case FIELD_TYPE::FLOAT_VALUE:
        if (colflags["field_type"]) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["field_type"])->Append("float"));

        if (colflags["value_integer"])
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int64Builder>(
                builders["value_integer"])->AppendNull());

        if (colflags["value_float"])
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::DoubleBuilder>(
                builders["value_float"])->Append(fval));

        if (colflags["value_string"]) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["value_string"])->Append(sval));
        break;
    
    case FIELD_TYPE::STRING_VALUE:
    default:
        if (colflags["field_type"]) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["field_type"])->Append("string"));

        if (colflags["value_integer"])
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::Int64Builder>(
                builders["value_integer"])->AppendNull());

        if (colflags["value_float"])
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::DoubleBuilder>(
                builders["value_float"])->AppendNull());

        if (colflags["value_string"]) 
            PARQUET_THROW_NOT_OK(std::dynamic_pointer_cast<arrow::StringBuilder>(
                builders["value_string"])->Append(sval));
    }
}

std::tuple<FIELD_TYPE, FIT_SINT64, FIT_FLOAT64> FitTransformer::_get_field_type(
    const fit::FieldBase& field, const std::string &sval, FIT_UINT8 j) {
    
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
    {
        // True integer only if NO scale/offset  
        // applied in fit::Field::GetFLOAT64Value
        FIT_FLOAT64 fval = field.GetFLOAT64Value(j);
        FIT_SINT64 ival = field.GetSINT64Value(j);
        
        static double TINY_NUMBER = .00000001;
        if (fabs(fval - ival) < TINY_NUMBER) 
            return std::make_tuple(FIELD_TYPE::INT_VALUE, ival, fval);
        else return std::make_tuple(FIELD_TYPE::FLOAT_VALUE, ival, fval);
        break;
    }

    case FIT_BASE_TYPE_FLOAT32:
    case FIT_BASE_TYPE_FLOAT64:
        return std::make_tuple(FIELD_TYPE::FLOAT_VALUE, 0, field.GetFLOAT64Value(j));
        break;

    case FIT_BASE_TYPE_STRING:
        return std::make_tuple(FIELD_TYPE::STRING_VALUE, 0, 0.0);
        break;
    
    default: 
        #if defined PYBIND11_PRINT_PYSTDOUT
        pybind11::print("Invalid FIT field datatype:", field.GetType());
        #else
        std::cerr << "Invalid FIT field datatype: " << field.GetType() << std::endl;
        #endif 

        return std::make_tuple(FIELD_TYPE::STRING_VALUE, 0, 0.0);
    }
}

std::shared_ptr<arrow::Schema> FitTransformer::_get_schema() 
{
    std::vector<std::shared_ptr<arrow::Field>> fldvec; // Last arg: true == nullable/optional, false == required
    if (CONFIG["source_filetype"] == "true") fldvec.push_back(arrow::field("source_filetype", arrow::utf8(), false));
    if (CONFIG["source_filename"] == "true") fldvec.push_back(arrow::field("source_filename", arrow::utf8(), false));
    if (CONFIG["source_file_uri"] == "true") fldvec.push_back(arrow::field("source_file_uri", arrow::utf8(), false));
    if (CONFIG["manufacturer_index"] == "true") fldvec.push_back(arrow::field("manufacturer_index", arrow::int32(), false));
    if (CONFIG["manufacturer_name"] == "true") fldvec.push_back(arrow::field("manufacturer_name", arrow::utf8(), false));
    if (CONFIG["product_index"] == "true") fldvec.push_back(arrow::field("product_index", arrow::int32(), false));
    if (CONFIG["product_name"] == "true") fldvec.push_back(arrow::field("product_name", arrow::utf8(), true));
    if (CONFIG["timestamp"] == "true") fldvec.push_back(arrow::field("timestamp", arrow::timestamp(arrow::TimeUnit::SECOND), true));
    if (CONFIG["mesg_index"] == "true") fldvec.push_back(arrow::field("mesg_index", arrow::int32(), false)); 
    if (CONFIG["mesg_name"] == "true") fldvec.push_back(arrow::field("mesg_name", arrow::utf8(), false)); 
    if (CONFIG["field_index"] == "true") fldvec.push_back(arrow::field("field_index", arrow::int32(), true));
    if (CONFIG["field_name"] == "true") fldvec.push_back(arrow::field("field_name", arrow::utf8(), false));
    if (CONFIG["field_type"] == "true") fldvec.push_back(arrow::field("field_type", arrow::utf8(), true));
    if (CONFIG["value_string"] == "true") fldvec.push_back(arrow::field("value_string", arrow::utf8(), false));
    if (CONFIG["value_integer"] == "true") fldvec.push_back(arrow::field("value_integer", arrow::int64(), true));
    if (CONFIG["value_float"] == "true") fldvec.push_back(arrow::field("value_float", arrow::float64(), true));
    if (CONFIG["units"] == "true") fldvec.push_back(arrow::field("units", arrow::utf8(), true));
    std::shared_ptr<arrow::Schema> p_schema;
    p_schema = arrow::schema(fldvec);
    return p_schema;
}

void FitTransformer::_init_from_config(std::unordered_map<std::string, bool> &cflags,
                                      std::unordered_map<std::string, bool> &exflags,
                                      std::unordered_map<std::string, pBuilder> &cbuilders) 
{
    // Set exclude flags
    exflags.insert({"exclude_empty_values", CONFIG["exclude_empty_values"] == "true"});
    exflags.insert({"exclude_timestamp_values", CONFIG["exclude_timestamp_values"] == "true"});

    // Set column flags
    for (int i = 0; i < colkeys.size(); ++i) cflags.insert({colkeys[i], CONFIG[colkeys[i]] == "true"});

    // Create column ArrayBuilders
    if (cflags["source_filetype"]) cbuilders.insert({"source_filetype", pBuilder(new arrow::StringBuilder())});
    if (cflags["source_filename"]) cbuilders.insert({"source_filename", pBuilder(new arrow::StringBuilder())});
    if (cflags["source_file_uri"]) cbuilders.insert({"source_file_uri", pBuilder(new arrow::StringBuilder())});
    if (cflags["manufacturer_index"]) cbuilders.insert({"manufacturer_index", pBuilder(new arrow::Int32Builder())});
    if (cflags["manufacturer_name"]) cbuilders.insert({"manufacturer_name", pBuilder(new arrow::StringBuilder())});
    if (cflags["product_index"]) cbuilders.insert({"product_index", pBuilder(new arrow::Int32Builder())});
    if (cflags["product_name"]) cbuilders.insert({"product_name", pBuilder(new arrow::StringBuilder())});
    if (cflags["timestamp"]) cbuilders.insert({"timestamp", pBuilder(new arrow::TimestampBuilder(
        arrow::timestamp(arrow::TimeUnit::SECOND), arrow::default_memory_pool()))});
    if (cflags["mesg_index"]) cbuilders.insert({"mesg_index", pBuilder(new arrow::Int32Builder())});
    if (cflags["mesg_name"]) cbuilders.insert({"mesg_name", pBuilder(new arrow::StringBuilder())});
    if (cflags["field_index"]) cbuilders.insert({"field_index", pBuilder(new arrow::Int32Builder())});
    if (cflags["field_name"]) cbuilders.insert({"field_name", pBuilder(new arrow::StringBuilder())});
    if (cflags["field_type"]) cbuilders.insert({"field_type", pBuilder(new arrow::StringBuilder())});
    if (cflags["value_string"]) cbuilders.insert({"value_string", pBuilder(new arrow::StringBuilder())});
    if (cflags["value_integer"]) cbuilders.insert({"value_integer", pBuilder(new arrow::Int64Builder())});
    if (cflags["value_float"]) cbuilders.insert({"value_float", pBuilder(new arrow::DoubleBuilder())});
    if (cflags["units"]) cbuilders.insert({"units", pBuilder(new arrow::StringBuilder())});
}

void FitTransformer::_write_parquet(const char parquet_fname[]) 
{
    // Finish builders into arrays
    std::vector<std::shared_ptr<arrow::Array>> tcolumns;
    for (int i = 0; i < colkeys.size(); ++i) {
        if (colflags[colkeys[i]]) {
            std::shared_ptr<arrow::Array> carray;
            PARQUET_THROW_NOT_OK(builders[colkeys[i]]->Finish(&carray));
            tcolumns.push_back(carray);
        }
    }
    
    // Make table from arrays, then write table to parquet outfile
    std::shared_ptr<::arrow::io::FileOutputStream> parquet_fhandle;
    std::shared_ptr<arrow::Table> atable_ptr = arrow::Table::Make(_get_schema(), tcolumns);
    PARQUET_ASSIGN_OR_THROW(parquet_fhandle, ::arrow::io::FileOutputStream::Open(parquet_fname));
    PARQUET_THROW_NOT_OK(parquet::arrow::WriteTable(*atable_ptr, arrow::default_memory_pool(), 
                                                    parquet_fhandle, ROW_GROUP_SIZE));
}

// Note: does NOT re-parse config file
void FitTransformer::_reset_state() {
    time_created = FIT_DATE_TIME_INVALID;
    manufacturer_index = FIT_MANUFACTURER_INVALID;
    product_index = FIT_UINT16_INVALID;
    source_filename.clear();
    source_file_uri.clear();
    manufacturer_name.clear();
    product_name.clear();

    for (auto bpair : builders) bpair.second->Reset();
}

int main(int argc, char* argv[])
{
   int retstatus = 1;
   if (argc == 3) {
        FitTransformer transformer;
        auto tstart = std::chrono::system_clock::now();
        retstatus = transformer.fit_to_parquet(argv[1], argv[2]);
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-tstart;
        if (retstatus == 0) std::cout << "Data transformation completed in " 
            << elapsed_seconds.count() << "sec" << std::endl;
   }
   else std::cerr << "Usage: fitparquet <fitfile> <parquetfile>" << std::endl;
   return retstatus;
}
