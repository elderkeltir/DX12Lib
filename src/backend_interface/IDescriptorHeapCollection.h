#pragma once

#include <optional>
#include <string>
#include "defines.h"

class IDescriptorHeapCollection {
public:
    virtual void Initialize(std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void ReserveRTVhandle(CPUdescriptor& rtvHandle) = 0;
    virtual void ReserveDSVhandle(CPUdescriptor& dsvHandle) = 0;
    virtual void ReserveSRVUAVCBVhandle(CPUdescriptor& srvuacbvHandle) = 0;
    virtual ~IDescriptorHeapCollection() = default;
};