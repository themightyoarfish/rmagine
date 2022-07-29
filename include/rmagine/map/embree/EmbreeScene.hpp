#ifndef RMAGINE_MAP_EMBREE_SCENE_HPP
#define RMAGINE_MAP_EMBREE_SCENE_HPP

#include "embree_types.h"
#include "EmbreeDevice.hpp"

#include <embree3/rtcore.h>
#include <memory>
#include <unordered_map>
#include <condition_variable>

namespace rmagine
{
struct EmbreeSceneSettings
{
    /**
     * @brief quality
     * - RTC_BUILD_QUALITY_LOW
     * - RTC_BUILD_QUALITY_MEDIUM
     * - RTC_BUILD_QUALITY_HIGH
     * - RTC_BUILD_QUALITY_REFIT
     * 
     */
    RTCBuildQuality quality = RTCBuildQuality::RTC_BUILD_QUALITY_MEDIUM;
    
    /**
     * @brief flags
     * 
     * - RTC_SCENE_FLAG_NONE (default)
     * - RTC_SCENE_FLAG_DYNAMIC
     * - RTC_SCENE_FLAG_COMPACT
     * - RTC_SCENE_FLAG_ROBUST
     * - RTC_SCENE_FLAG_CONTEXT_FILTER_FUNCTION
     */
    RTCSceneFlags flags = RTCSceneFlags::RTC_SCENE_FLAG_NONE;
};

/**
 * @brief EmbreeScene
 * 
 * - meshes
 * - instances
 * 
 */
class EmbreeScene
: public std::enable_shared_from_this<EmbreeScene>
{
public:
    // EmbreeScene(
    //     EmbreeDevicePtr device = embree_default_device());

    EmbreeScene(
        EmbreeSceneSettings settings = {}, 
        EmbreeDevicePtr device = embree_default_device());
        
    ~EmbreeScene();

    void setQuality(RTCBuildQuality quality);

    void setFlags(RTCSceneFlags flags);

    unsigned int add(EmbreeGeometryPtr geom);
    EmbreeGeometryPtr get(unsigned int geom_id) const;
    std::unordered_map<unsigned int, EmbreeGeometryPtr> geometries() const;
    bool has(unsigned int geom_id) const;
    EmbreeGeometryPtr remove(unsigned int geom_id);

    template<typename T>
    std::shared_ptr<T> get_as(unsigned int geom_id) const;

    template<typename T>
    unsigned int count() const;


    RTCScene handle();

    void commit();

    /**
     * @brief 
     * 
     */
    void optimize();

    // Scene has no right to let parents stay alive
    EmbreeInstanceWSet parents;
private:

    std::unordered_map<unsigned int, EmbreeGeometryPtr > m_geometries;

    RTCScene m_scene;
    EmbreeDevicePtr m_device;
};


template<typename T>
unsigned int EmbreeScene::count() const
{
    unsigned int ret = 0;

    for(auto it = m_geometries.begin(); it != m_geometries.end(); ++it)
    {
        if(std::dynamic_pointer_cast<T>(it->second))
        {
            ret++;
        }
    }

    return ret;
}


template<typename T>
std::shared_ptr<T> EmbreeScene::get_as(unsigned int geom_id) const
{
    std::shared_ptr<T> ret;

    EmbreeGeometryPtr geom = get(geom_id);
    if(geom)
    {
        ret = std::dynamic_pointer_cast<T>(geom);
    }

    return ret;
}


} // namespace rmagine

#endif // RMAGINE_MAP_EMBREE_SCENE_HPP