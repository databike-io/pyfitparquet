#include "fitparquet.h"
#include <pybind11/pybind11.h>

// TODO: add cout/cerr redirects to python's sys.stdout, sys.stderr
// See: https://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html#capturing-standard-output-from-ostream

PYBIND11_MODULE(pyfitparquet_so, m) {
    pybind11::class_<FPTransformer>(m, "FPTransformer")
        .def(pybind11::init<>())
        .def("fit_to_parquet", &FPTransformer::fit_to_parquet)
        .def("reset_from_config", &FPTransformer::reset_from_config);
}
