#ifndef IMAGINE_SIMULATION_OPTIX_O1DN_PROGRAM_GENERIC_HPP
#define IMAGINE_SIMULATION_OPTIX_O1DN_PROGRAM_GENERIC_HPP

#include <rmagine/map/OptixMap.hpp>
#include <rmagine/util/optix/OptixProgram.hpp>
#include <rmagine/simulation/optix/OptixSimulationData.hpp>

namespace rmagine {

class O1DnProgramGeneric : public OptixProgram
{
public:
    O1DnProgramGeneric(
        OptixMapPtr map,
        const OptixSimulationDataGenericO1Dn& flags);
};

} // namespace rmagine

#endif // IMAGINE_SIMULATION_OPTIX_O1DN_PROGRAM_GENERIC_HPP