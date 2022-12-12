#pragma once
#include "LevelEntity.h"

class GpuResource;

class SkyBox : public LevelEntity {
public:
    SkyBox();
    void Load(const std::wstring &name) override;
    GpuResource* GetTexture();
};