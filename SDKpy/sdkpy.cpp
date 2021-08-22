#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sdk.h"

namespace py = pybind11;

int WxSetTextMsgCbPy(const std::function<int(WxMessage_t)> &onMsg) { return WxSetTextMsgCb(onMsg); }

PYBIND11_MODULE(wcferry, m)
{
    m.doc() = "SDK python API";

    py::class_<WxMessage>(m, "WxMessage")
        .def_readonly("id", &WxMessage::id)
        .def_readonly("self", &WxMessage::self)
        .def_readonly("type", &WxMessage::type)
        .def_readonly("source", &WxMessage::source)
        .def_readonly("xml", &WxMessage::xml)
        .def_readonly("wxId", &WxMessage::wxId)
        .def_readonly("roomId", &WxMessage::roomId)
        .def_readonly("content", &WxMessage::content);

    m.def("WxInitSDK", &WxInitSDK);
    m.def("WxSetTextMsgCb", &WxSetTextMsgCbPy);
    m.def("WxSendTextMsg", &WxSendTextMsg);
    m.def("WxSendImageMsg", &WxSendImageMsg);
    m.def("WxGetMsgTypes", &WxGetMsgTypes, py::return_value_policy::reference);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
