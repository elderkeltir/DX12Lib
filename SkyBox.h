#pragma once
#include "LevelEntity.h"

class SkyBox : public LevelEntity {
public:
    SkyBox();
    void Load(const std::wstring &name) override;
};