#include "fitparquet.h"

#if defined BIND_WITH_BOOST_PYTHON

    #include <boost/python.hpp>
    using namespace boost::python;

    BOOST_PYTHON_MODULE(pyfitparquet) {
        class_<FPTransformer>("FPTransformer", init<>())
            .def("fit_to_parquet", &FPTransformer::fit_to_parquet);
    }

#else

    #include <pybind11/pybind11.h>

    PYBIND11_MODULE(pyfitparquet, m) {
        pybind11::class_<FPTransformer>(m, "FPTransformer")
            .def("fit_to_parquet", &FPTransformer::fit_to_parquet);
    }

    // PYBIND11_MODULE(pyfitparquet, m) {
    //     pybind11::class_<FPTransformer>(m, "FPTransformer")
    //         .def(pybind11::init<>())
    //         .def("fit_to_parquet", &FPTransformer::fit_to_parquet);
    // }

#endif 
