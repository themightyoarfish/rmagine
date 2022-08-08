#ifndef RMAGINE_SIMULATION_OPTIX_ONDN_PROGRAM_GENERIC_HPP
#define RMAGINE_SIMULATION_OPTIX_ONDN_PROGRAM_GENERIC_HPP

#include <rmagine/map/OptixMap.hpp>
#include <rmagine/util/optix/OptixProgram.hpp>
#include <rmagine/simulation/optix/OptixSimulationData.hpp>

#include <rmagine/util/optix/OptixSbtRecord.hpp>
#include <rmagine/map/optix/optix_definitions.h>

namespace rmagine {

class OnDnProgramGeneric : public OptixProgram
{
    using RayGenData        = RayGenDataEmpty;
    using MissData          = MissDataEmpty;
    using HitGroupData      = HitGroupDataScene;

    using RayGenSbtRecord   = SbtRecord<RayGenData>;
    using MissSbtRecord     = SbtRecord<MissData>;
    using HitGroupSbtRecord = SbtRecord<HitGroupData>;
public:
    OnDnProgramGeneric(
        OptixMapPtr map,
        const OptixSimulationDataGenericOnDn& flags);

    virtual ~OnDnProgramGeneric();

    void updateSBT();

private:
    // scene container
    OptixMapPtr         m_map;
    // currently used scene
    OptixScenePtr       m_scene;

    RayGenSbtRecord     rg_sbt;
    MissSbtRecord       ms_sbt;
    HitGroupSbtRecord   hg_sbt;
};

} // namespace rmagine

#endif // RMAGINE_SIMULATION_OPTIX_ONDN_PROGRAM_GENERIC_HPP