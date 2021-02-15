#include "fittransformer.h"
#include <pybind11/pybind11.h>


PYBIND11_MODULE(fittransformer_so, m) {
    pybind11::class_<FitTransformer>(m, "FitTransformer")
        .def(pybind11::init<>())
        .def("fit_to_parquet", &FitTransformer::fit_to_parquet)
        .def("reset_from_config", &FitTransformer::reset_from_config);
}
