#include <fstream>
#include <iostream>

#include "fit_decode.hpp"
#include "fit_mesg_broadcaster.hpp"
//#include "fit_developer_field_description.hpp"

#include <arrow/io/file.h>
#include <arrow/util/logging.h>
#include <parquet/api/reader.h>
#include <parquet/api/writer.h>


using parquet::Type;
using parquet::Repetition;
using parquet::schema::GroupNode;
using parquet::schema::PrimitiveNode;
using PqFileStream = ::arrow::io::FileOutputStream;


class ParquetRow {
public:

    static std::shared_ptr<GroupNode> get_schema() {
        static std::shared_ptr<GroupNode> p_ourschema;
        if (!p_ourschema) {
            parquet::schema::NodeVector fields;
            fields.push_back(PrimitiveNode::Make("manufacturer",  Repetition::REQUIRED, Type::INT32));
            fields.push_back(PrimitiveNode::Make("product",       Repetition::REQUIRED, Type::INT32));
            fields.push_back(PrimitiveNode::Make("timestamp",     Repetition::REQUIRED, Type::INT64));
            fields.push_back(PrimitiveNode::Make("mesg_index",    Repetition::REQUIRED, Type::INT32));
            fields.push_back(PrimitiveNode::Make("mesg_name",     Repetition::REQUIRED, Type::BYTE_ARRAY));
            fields.push_back(PrimitiveNode::Make("field_type",    Repetition::REQUIRED, Type::BYTE_ARRAY));
            fields.push_back(PrimitiveNode::Make("field_name",    Repetition::REQUIRED, Type::BYTE_ARRAY));
            fields.push_back(PrimitiveNode::Make("value_string",  Repetition::REQUIRED, Type::BYTE_ARRAY));
            fields.push_back(PrimitiveNode::Make("value_numeric", Repetition::OPTIONAL, Type::DOUBLE));
            fields.push_back(PrimitiveNode::Make("mesg_name",     Repetition::OPTIONAL, Type::BYTE_ARRAY)); 
            p_ourschema = std::static_pointer_cast<GroupNode>(GroupNode::Make("schema", Repetition::REQUIRED, fields));
        }

        return p_ourschema;
    }

    ParquetRow(FIT_MANUFACTURER manufacturer_a, FIT_UINT16 product_a, 
        FIT_UINT16 mesg_index_a, std::string mesg_name_a) 
        : manufacturer((std::int32_t)manufacturer_a), product((std::int32_t)product_a), 
          mesg_index((std::int32_t)mesg_index_a), mesg_name(mesg_name_a) {}

    void set_timestamp(FIT_DATE_TIME timestamp_a)     { timestamp = (std::int64_t)timestamp_a; }
    void set_field_type(std::string field_type_a)     { field_type = field_type_a; }
    void set_field_name(std::string field_name_a)     { field_name = field_name_a; }
    void set_value_string(std::wstring value_string_a) { value_string = value_string_a; }
    void set_units(std::string units_a)               { units = units_a; }

    void set_value_numeric(FIT_FLOAT64 value_numeric_a) { 
        value_numeric = (double)value_numeric_a;
        set_value_string(std::to_wstring(value_numeric_a));
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
            set_value_numeric(field.GetFLOAT64Value(j));
            set_field_type("numeric");
            break;
        case FIT_BASE_TYPE_STRING:
            set_value_string(field.GetSTRINGValue(j));
            break;
        default: break;
        }
    }

    void write_parquet_row(parquet::RowGroupWriter* rgroup_writer) {

    }

private:

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
    double value_numeric;
    std::wstring value_string;
    std::string units;
};

class ParquetSerializer : public fit::MesgListener 
{
public:

    ParquetSerializer() : 
        time_created(FIT_DATE_TIME_INVALID),
        manufacturer(FIT_MANUFACTURER_INVALID),
        product(FIT_UINT16_INVALID) {}

    int fit_to_parquet(const char fit_filename[], const char parquet_filename[]) 
    {
        try {
            // Open FIT file
            fit_fhandle.open(fit_filename, std::ios::in | std::ios::binary);
            if (!fit_fhandle.is_open()) throw std::runtime_error(
                std::string("ERROR opening FIT file: ") + fit_filename);

            // Validate FIT file
            if (!fit_decoder.CheckIntegrity(fit_fhandle)) throw std::runtime_error(
                std::string("Integrity FAILURE for FIT file: ") + fit_filename);

            // Add FIT message listener (facilitates OnMesg callback)
            msg_broadcaster.AddListener((fit::MesgListener &)*this);

            // Open parquet output file
            PARQUET_ASSIGN_OR_THROW(parquet_fhandle, PqFileStream::Open(parquet_filename));

            // Create parquet writer properties
            parquet::WriterProperties::Builder builder;
            //builder.compression(parquet::Compression::SNAPPY);
            std::shared_ptr<parquet::WriterProperties> props = builder.build();

            // Open ParquetFileWriter instance
            std::shared_ptr<GroupNode> schema = ParquetRow::get_schema();
            std::shared_ptr<parquet::ParquetFileWriter> parquet_writer =
                parquet::ParquetFileWriter::Open(parquet_fhandle, schema, props);

            // Append RowGroup instance to ParquetFileWriter.
            parquet::RowGroupWriter* rgroup_writer = parquet_writer->AppendRowGroup();

            // Execute FIT-to-parquet translation/serialization 
            fit_decoder.Read(fit_fhandle, msg_broadcaster);
            parquet_writer->Close(); // Close the ParquetFileWriter
            DCHECK(parquet_fhandle->Close().ok()); // Write the bytes to file
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
                std::cout << "  Dropping invalid mesg: " << mesg.GetName() << std::endl;
                break;
            
            case FIT_MESG_NUM_FILE_ID:
            {
                fit::FileIdMesg& fid_mesg = static_cast<fit::FileIdMesg&>(mesg); // Downcast from non-virtual base
                if (fid_mesg.IsTimeCreatedValid() == FIT_TRUE) time_created = fid_mesg.GetTimeCreated();
                if (fid_mesg.IsManufacturerValid() == FIT_TRUE) manufacturer = fid_mesg.GetManufacturer();
                if (fid_mesg.IsProductValid() == FIT_TRUE) product = fid_mesg.GetProduct();
            } // Fall thru to default

            default:
            {
                if (manufacturer != FIT_MANUFACTURER_INVALID && product != FIT_UINT16_INVALID)
                {
                    std::vector<ParquetRow> pqrows;
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

                    // Finalize rows and write'em to RowGroupWriter
                    for (int i = 0; i < pqrows.size(); ++i) {
                        pqrows[i].set_timestamp(timestamp_a);
                        //pqrows[i].write_parquet_row(rgroup_writer);
                    }
                }
                else std::cerr << "  Manufacturer/Product invalid, dropping: " << mesg.GetName() << std::endl;
            }
        }
    }

private:

   // Fit input
   fit::Decode fit_decoder;
   std::fstream fit_fhandle;
   fit::MesgBroadcaster msg_broadcaster;
   
   // Fit file level (primary-ish key)
   FIT_DATE_TIME time_created;
   FIT_MANUFACTURER manufacturer;
   FIT_UINT16 product;

   // Parquet output
   std::shared_ptr<PqFileStream> parquet_fhandle;
   std::shared_ptr<parquet::ParquetFileWriter> parquet_writer;
   parquet::RowGroupWriter* rgroup_writer;

};

int main(int argc, char* argv[])
{
   int retstatus = 1;
   if (argc == 3) {
        ParquetSerializer pserializer;
        retstatus = pserializer.fit_to_parquet(argv[1], argv[2]);
        if (retstatus == 0) std::cout << "Serialization completed: "
            << argv[1] << " to " << argv[2];
   }
   else std::cerr << "Usage: serializer <fitfile> <parquetfile>" << std::endl;
   return retstatus;
}
