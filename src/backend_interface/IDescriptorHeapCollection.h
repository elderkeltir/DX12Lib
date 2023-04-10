#pragma once

#include <optional>
#include <string>
#include "defines.h"

class IDescriptorHeapCollection {
public:
    virtual void Initialize(std::optional<std::wstring> dbg_name = std::nullopt) = 0;
    virtual void ReserveRTVhandle(CPUdescriptor& rtvHandle, uint64_t data = 0, bool gpu_only = false) = 0;
    virtual void ReserveDSVhandle(CPUdescriptor& dsvHandle, uint64_t data = 0, bool gpu_only = false) = 0;
    virtual void ReserveCBVhandle(CPUdescriptor& cbvHandle, uint64_t data = 0, bool gpu_only = false) = 0;
    virtual void ReserveSRVhandle(CPUdescriptor& srvHandle, uint64_t data = 0, bool gpu_only = false) = 0;
    virtual void ReserveUAVhandle(CPUdescriptor& uavHandle, uint64_t data = 0, bool gpu_only = false) = 0;
    virtual ~IDescriptorHeapCollection() = default;
};