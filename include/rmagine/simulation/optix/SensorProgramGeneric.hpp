#ifndef RMAGINE_SIMULATION_OPTIX_SENSOR_PROGRAM_GENERIC_HPP
#define RMAGINE_SIMULATION_OPTIX_SENSOR_PROGRAM_GENERIC_HPP

#include <rmagine/map/OptixMap.hpp>
#include <rmagine/util/optix/OptixProgram.hpp>
#include <rmagine/simulation/optix/OptixSimulationData.hpp>

#include <rmagine/util/optix/OptixSbtRecord.hpp>
#include <rmagine/map/optix/optix_definitions.h>
#include <rmagine/map/optix/optix_sbt.h>

#include <memory>

#include "OptixProgramMap.hpp"

namespace rmagine {

class SensorProgramGeneric : public OptixProgram
{
    using RayGenData        = RayGenDataEmpty;
    using MissData          = MissDataEmpty;
    using HitGroupData      = OptixSceneSBT;

    using RayGenSbtRecord   = SbtRecord<RayGenData>;
    using MissSbtRecord     = SbtRecord<MissData>;
    using HitGroupSbtRecord = SbtRecord<HitGroupData>;

public:
    SensorProgramGeneric(
        OptixMapPtr map,
        const OptixSimulationDataGeneric& flags);

    SensorProgramGeneric(
        OptixScenePtr scene,
        const OptixSimulationDataGeneric& flags);

    virtual ~SensorProgramGeneric();

    void updateSBT();

    OptixSimulationDataGeneric flags;

private:
    // currently used scene
    OptixScenePtr       m_scene;

    OptixModule         module_gen;
    OptixModule         module_hit;

    Memory<RayGenSbtRecord, RAM> rg_sbt;
    Memory<MissSbtRecord, RAM> ms_sbt;
    Memory<HitGroupSbtRecord, RAM> hg_sbt;
};

using SensorProgramGenericPtr = std::shared_ptr<SensorProgramGeneric>;

} // namespace rmagine

namespace std
{

template<>
struct hash<rmagine::SensorProgramGenericPtr >
{
    std::size_t operator()(const rmagine::SensorProgramGenericPtr& k) const
    {
        return hash<rmagine::OptixSimulationDataGeneric>()(k->flags);
    }
};

} // namespace std

#endif // RMAGINE_SIMULATION_OPTIX_SENSOR_PROGRAM_GENERIC_HPP