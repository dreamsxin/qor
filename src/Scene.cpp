#include "Scene.h"
#include "Mesh.h"
#include "Sound.h"
#include "Light.h"
#include "Material.h"
#include "kit/meta/meta.h"
using namespace std;

Scene :: Scene(const string& fn, Cache<Resource, std::string>* cache):
    //Resource(fn),
    m_pConfig(make_shared<Meta>(fn)),
    m_Filename(fn),
    m_pCache(cache),
    m_pRoot(make_shared<Node>())
    //m_pData(make_shared<Meta>(fn))
{
    load();
}

void Scene :: iterate_data(const std::shared_ptr<Meta>& doc)
{
    string name = doc->at<string>("name", string());
    string type = doc->at<string>("type", string());
    LOGf("data: %s: %s", name % type);
    try{
        if(type == "mesh")
            m_pCache->cache_cast<Mesh::Data>(m_Filename + ":" + doc->at<string>("name"));
        else if(type == "material")
            m_pCache->cache_cast<Material>(doc->at<string>("image"));
    }catch(const Error& e){
    }catch(const std::exception& e){
        WARNING(e.what());
    }
}

void Scene :: iterate_node(const std::shared_ptr<Node>& parent, const std::shared_ptr<Meta>& doc)
{
    shared_ptr<Node> node;
    string name = doc->at<string>("name", string());
    string type = doc->at<string>("type", string());
    LOGf("node: %s: %s", name % type);
    
    // TODO: use node factory instead
    
    if(type == "empty")
        node = make_shared<Node>();
    else if(type == "mesh")
    {
        LOG("mesh");
        //string data = doc->at<string>("data", string());
        node = make_shared<Mesh>(
            m_pCache->cache_cast<Mesh::Data>(m_Filename + ":" + doc->at<string>("data"))
            //m_pCache->cache_cast<Mesh::Data>(m_Filename + ":" + doc->at<string>("name"))
        );
    }
    else if(type == "sound")
    {
        LOGf("sound: %s", doc->at<string>("sound"));
        auto snd = make_shared<Sound>(
            doc->at<string>("sound"),
            m_pCache
        );
        snd->source()->play();
        snd->source()->update();
        node = snd;
    }
    else if(type == "light")
    {
        LOG("light");
        auto light = make_shared<Light>(doc);
        auto color = doc->at<shared_ptr<Meta>>("color", make_shared<Meta>());
        if(not color->empty())
            light->diffuse(Color(
                (float)color->at<double>(0),
                (float)color->at<double>(1),
                (float)color->at<double>(2)
            ));
        node = light;
    }
    
    if(not node)
        node = make_shared<Node>();
    
    node->name(name);
    
    auto mat = doc->at<shared_ptr<Meta>>("matrix", make_shared<Meta>());
    if(not mat->empty())
    {
        *node->matrix() = glm::mat4(
            mat->at<double>(0),
            mat->at<double>(1),
            mat->at<double>(2),
            mat->at<double>(3),
            mat->at<double>(4),
            mat->at<double>(5),
            mat->at<double>(6),
            mat->at<double>(7),
            mat->at<double>(8),
            mat->at<double>(9),
            mat->at<double>(10),
            mat->at<double>(11),
            mat->at<double>(12),
            mat->at<double>(13),
            mat->at<double>(14),
            mat->at<double>(15)
        );
    }
    
    parent->add(node);
    node->pend();
    
    try{
        for(auto& e: *doc->meta("nodes"))
        {
            try{
                iterate_node(node, e.as<std::shared_ptr<Meta>>());
            }catch(const boost::bad_any_cast&){}
        }
    }catch(const out_of_range&){}
}


void Scene :: load()
{
    auto grav = m_pConfig->meta("gravity", make_shared<Meta>(
        MetaFormat::JSON, "[0.0, -9.8, 0.0]"
    ));
    m_Gravity = glm::vec3(
        grav->at<double>(0),
        grav->at<double>(1),
        grav->at<double>(2)
    );
    //for(auto& e: *m_pConfig->meta("data"))
    //{
    //    try{
    //        iterate_data(e.as<std::shared_ptr<Meta>>());
    //    }catch(const boost::bad_any_cast&){}
    //}
    m_pRoot = make_shared<Node>();
    for(auto& e: *m_pConfig->meta("nodes"))
    {
        try{
            iterate_node(m_pRoot, e.as<std::shared_ptr<Meta>>());
        }catch(const boost::bad_any_cast&){}
    }
}

