#pragma once
#include "LevelEntity.h"

class IGpuResource;

class SkyBox : public LevelEntity {
public:
    SkyBox();
    void Load(const std::wstring &name) override;
    IGpuResource* GetTexture();
};