#ifndef RMAGINE_MAP_EMBREE_INSTANCE_HPP
#define RMAGINE_MAP_EMBREE_INSTANCE_HPP

#include "embree_definitions.h"

#include <rmagine/math/types.h>
#include <rmagine/types/mesh_types.h>
#include <unordered_set>
#include <memory>

#include <embree3/rtcore.h>

#include <functional>

#include "EmbreeDevice.hpp"
#include "EmbreeGeometry.hpp"

namespace rmagine
{

/**
 * @brief EmbreeInstance
 * 
 * N instances belongs to 1 scene
 * M instances of 1 mesh
 * 
 * N instance belongs to 1 parent scene
 * 1 instance has one child scene
 * 
 */
class EmbreeInstance
: public EmbreeGeometry
{
public:
    using Base = EmbreeGeometry;

    EmbreeInstance(EmbreeDevicePtr device = embree_default_device() );
    virtual ~EmbreeInstance();

    void set(EmbreeScenePtr scene);
    EmbreeScenePtr scene();

    // Make this more comfortable to use
    // - functions as: setMesh(), or addMesh() ?
    // - translate rotate scale? 

    /**
     * @brief Apply transformation. Need to commit afterwards
     * 
     */
    void apply();

    virtual EmbreeGeometryType type() const
    {
        return EmbreeGeometryType::INSTANCE;
    }

private:

    // scene that is instanced by this object
    EmbreeScenePtr m_scene;
};

} // namespace rmagine

#endif // RMAGINE_MAP_EMBREE_INSTANCE_HPP