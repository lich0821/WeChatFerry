#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "sdk.h"

namespace py = pybind11;

int WxEnableRecvMsgPy(const std::function<int(WxMessage_t)>& onMsg) { return WxEnableRecvMsg(onMsg); }

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

    py::class_<WxContact>(m, "WxContact")
        .def_readonly("wxId", &WxContact::wxId)
        .def_readonly("wxCode", &WxContact::wxCode)
        .def_readonly("wxName", &WxContact::wxName)
        .def_readonly("wxCountry", &WxContact::wxCountry)
        .def_readonly("wxProvince", &WxContact::wxProvince)
        .def_readonly("wxCity", &WxContact::wxCity)
        .def_readonly("wxGender", &WxContact::wxGender);

    py::class_<WxDbTable>(m, "WxDbTable")
        .def_readonly("table", &WxDbTable::table)
        .def_readonly("sql", &WxDbTable::sql);

    m.def("WxInitSDK", &WxInitSDK, "Initiate SDK. Return 0 on success，else on failure.");
    m.def("WxEnableRecvMsg", &WxEnableRecvMsgPy, "Enable message receiving and provide a callback", py::arg("onMsg"));
    m.def("WxDisableRecvMsg", &WxDisableRecvMsg, "Disable message receiving.");
    m.def("WxSendTextMsg", &WxSendTextMsg, "Send text message.", py::arg("wxid"), py::arg("msg"), py::arg("atWxids")=L"");
    m.def("WxSendImageMsg", &WxSendImageMsg, "Send image message.", py::arg("wxid"), py::arg("path"));
    m.def("WxGetContacts", &WxGetContacts, py::return_value_policy::reference, "Get contact list.");
    m.def("WxGetMsgTypes", &WxGetMsgTypes, py::return_value_policy::reference, "Get message types.");
    m.def("WxGetDbNames", &WxGetDbNames, py::return_value_policy::reference, "Get DB names.");
    m.def("WxGetDbTables", &WxGetDbTables, py::return_value_policy::reference, "Get DB tables.", py::arg("db"));

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
