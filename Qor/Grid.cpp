#include <vector>
#include <algorithm>
#include <memory>
#include "Grid.h"
#include "Mesh.h"
using namespace std;
using namespace glm;

Grid :: Grid(){}

void Grid :: add_tile(std::shared_ptr<Node> n, glm::ivec2 loc)
{
    unsigned ofs = loc.y*m_Size.x+loc.x;
    //LOGf("ofs: %s", ofs);
    //LOGf("ts: %s", m_Tiles.size());
    //assert(m_Tiles.size() > ofs);
    if(m_Tiles.size() <= ofs)
        m_Tiles.resize(ofs + 1);
    //add(n);
    m_Tiles[ofs] = n;
    n->_set_parent(this); // hack: we want to fake this relationship so the
                          // transform comes out right
}

void Grid :: remove_tile(Node* tile)
{
    kit::remove(m_Tiles, tile->as_node());
}

std::vector<Node*> Grid :: query(Box box, std::function<bool(Node*)> cond)
{
    auto r = std::vector<Node*>();
    int xs = box.min().x / m_TileSize.x;
    int ys = box.min().y / m_TileSize.y;
    int xe = box.max().x / m_TileSize.x + 1;
    int ye = box.max().y / m_TileSize.y + 1;
    for(int j=ys; j<ye; ++j)
        for(int i=xs; i<xe; ++i)
        {
            auto tile = ((Grid*)this)->tile(i,j).get();
            if(tile){
                if(not cond || cond(tile))
                    r.push_back(tile);
                auto desc = tile->descendants();
                std::copy_if(ENTIRE(desc), back_inserter(r), [cond](Node* n){
                    return not cond || cond(n);
                });
            }
        }
    return r;
}

std::vector<const Node*> Grid :: visible_nodes(Camera* camera) const
{
    if(m_pTemp)
        return std::vector<const Node*>();
    //if(m_bDirty)
    //{
        int xs = (camera->ortho_frustum().min().x - m_Range) / m_TileSize.x;
        int ys = (camera->ortho_frustum().min().y - m_Range) / m_TileSize.y;
        int xe = (camera->ortho_frustum().max().x + m_Range) / m_TileSize.x + 1;
        int ye = (camera->ortho_frustum().max().y + m_Range) / m_TileSize.y + 1;
        std::vector<const Node*> r((xe-xs)*(ye-ys)); // const
        for(int j=ys; j<ye; ++j)
            for(int i=xs; i<xe; ++i){
                auto tile = ((Grid*)this)->tile(i,j).get();
                if(tile && tile->visible()&& tile->self_visible() && camera->is_visible_func(tile,nullptr)){
                    r.push_back(tile);
                    auto desc = tile->descendants();
                    std::copy_if(ENTIRE(desc), back_inserter(r), [camera](const Node* n){
                        return n->visible() && n->self_visible() && camera->is_visible_func(n,nullptr);
                    });
                }
            }
        //return r;
        //if(not m_pTemp)
        //    m_pTemp = std::make_shared<Node>();
        //Mesh::bake(m_pTemp, r, nullptr, [](Node*){ return true; });
        //m_pTemp->_set_parent((Node*)this);
        //m_bDirty = false;
    //}
    //std::vector<const Node*> r2;
    //if(m_pTemp.get())
    //    r2.push_back(m_pTemp.get());
    return r;
}

bool Grid :: bake_visible()
{
    //if(m_Dirty)
    //{
        if(m_pTemp){
            m_pTemp->detach();
            m_pTemp = nullptr;
        }
        auto cnodes = visible_nodes(m_pMainCamera);
        vector<Node*> nodes;
        std::transform(ENTIRE(cnodes), back_inserter(nodes), [](const Node* n){
            return (Node*)n;
        });
        m_pTemp = std::make_shared<Node>();
        Mesh::bake(m_pTemp, nodes, nullptr, [](Node*){ return true; });
        m_pTemp->_set_parent((Node*)this);
        add(m_pTemp);
        
        //m_Dirty = false;
        return true;
    //}
    
    //return false;
}

std::vector<Node*> Grid :: all_descendants()
{
    std::vector<Node*> r;
    std::transform(ENTIRE(m_Tiles), back_inserter(r), [](shared_ptr<Node> n){
        return n.get();
    });
    auto desc = descendants();
    std::copy(ENTIRE(desc), back_inserter(r));
    return r;
}

void Grid :: logic_self(Freq::Time t)
{
    Node::logic_self(t);
    if(m_pMainCamera)
        for(auto&& tile: visible_nodes(m_pMainCamera)){
            if(not tile)
                continue;
            const_cast<Node*>(tile)->lazy_logic(t);
        }
}

void Grid :: render_self(Pass* pass) const
{
    //Node::render_self(pass);
    //for(auto&& tile: m_Tiles){
    //for(m_Tiles)
    //    if(not tile)
    //        continue;
    //    if(pass->camera()->is_node_visible_func(tile))
    //        tile->render_self(pass);
    //}
}

std::shared_ptr<Node> Grid :: tile(int x, int y)
{
    try{
        return m_Tiles.at(y*m_Size.x+x);
    }catch(const std::out_of_range&){
        return nullptr;
    }
}

