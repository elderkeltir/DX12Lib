#include "SkyBox.h"
#include <memory>
#include <filesystem>
#include "Level.h"
//#include "DXAppImplementation.h"

#if defined(min)
#undef min
#endif
 
#if defined(max)
#undef max
#endif
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <fstream>

#include "Frontend.h"
#include "RenderModel.h"
#include "FileManager.h"
#include "Level.h"

extern Frontend *gFrontend;
using rapidjson::Document;
using rapidjson::Value;

SkyBox::SkyBox() : LevelEntity(DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(0,0,0), DirectX::XMFLOAT3(200,200,200))
{
    Update(0.f);
}

void SkyBox::Load(const std::wstring &name) {
    // read file
    std::string content;
    {
        if (std::shared_ptr<Level> level =  gFrontend->GetLevel().lock()){
            const std::filesystem::path fullPath = (level->GetEntitiesDir() / name);
            std::ifstream ifs(fullPath.wstring());
            content.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        }
    }

    // parse file
    Document d;
    d.Parse(content.c_str());

    const Value &shaders = d["technique"];
    m_tech_id = shaders.GetInt();

    const char * tex_name_8 = d["texture"].GetString();
    const std::wstring tex_name(&tex_name_8[0], &tex_name_8[strlen(tex_name_8)]);
    if (std::shared_ptr<FileManager> fileMgr = gFrontend->GetFileManager().lock()){
        RenderObject * model = nullptr;
        fileMgr->CreateModel(tex_name, FileManager::Geom_type::gt_sphere, model);
        m_model = (RenderModel*)model;
        m_model->SetName(L"SkyBox");
        m_model->SetTechniqueId(m_tech_id);
    }
}

IGpuResource* SkyBox::GetTexture() {
	return m_model->GetTexture(RenderObject::TextureType::DiffuseTexture);
}
