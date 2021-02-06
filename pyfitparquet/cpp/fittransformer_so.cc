#include "fittransformer.h"
#include <pybind11/pybind11.h>

// TODO: add cout/cerr redirects to python's sys.stdout, sys.stderr
// See: https://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html#capturing-standard-output-from-ostream

PYBIND11_MODULE(fittransformer_so, m) {
    pybind11::class_<FitTransformer>(m, "FitTransformer")
        .def(pybind11::init<>())
        .def("fit_to_parquet", &FitTransformer::fit_to_parquet)
        .def("reset_from_config", &FitTransformer::reset_from_config);
}
