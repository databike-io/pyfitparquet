#include "fitparquet.h"
#include <pybind11/pybind11.h>

// TODO: add cout/cerr redirects to python's sys.stdout, sys.stderr
// See: https://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html#capturing-standard-output-from-ostream

PYBIND11_MODULE(pyfitparquet, m) {
    pybind11::class_<FPTransformer>(m, "FPTransformer")
        .def("fit_to_parquet", &FPTransformer::fit_to_parquet);
}
