#ifndef IMAGINE_SIMULATION_OPTIX_SIMULATION_DATA_HPP
#define IMAGINE_SIMULATION_OPTIX_SIMULATION_DATA_HPP

#include <imagine/util/optix/OptixData.hpp>
#include <imagine/types/sensor_models.h>
#include <imagine/math/types.h>
#include <imagine/types/MemoryCuda.hpp>

#include <optix.h>
#include <cuda_runtime.h>

namespace imagine
{

template<typename ModelT>
struct OptixSimulationDataRanges {
    // Input
    const Transform*            Tsb; // Static offset of sensor
    const ModelT*               model; // Scanner Model
    const Transform*            Tbm; // Poses
    uint32_t                    Nposes;
    // Handle
    OptixTraversableHandle      handle;
    // Result
    float*                      ranges;
};

using OptixSimulationDataRangesSphere = OptixSimulationDataRanges<SphericalModel>;
using OptixSimulationDataRangesPinhole = OptixSimulationDataRanges<PinholeModel>;
using OptixSimulationDataRangesO1Dn = OptixSimulationDataRanges<O1DnModel<VRAM_CUDA> >;

template<typename ModelT>
struct OptixSimulationDataNormals {
    // Input
    const Transform*            Tsb; // Static offset of sensor
    const ModelT*               model; // Scanner Model
    const Transform*            Tbm; // Poses
    uint32_t                    Nposes;
    // Handle
    OptixTraversableHandle      handle;
    // Result
    Vector*                     normals;
};

using OptixSimulationDataNormalsSphere = OptixSimulationDataNormals<SphericalModel>;
using OptixSimulationDataNormalsPinhole = OptixSimulationDataNormals<PinholeModel>;
using OptixSimulationDataNormalsO1Dn = OptixSimulationDataNormals<O1DnModel<VRAM_CUDA> >;


template<typename ModelT>
struct OptixSimulationDataGeneric {
    // Input
    const Transform*            Tsb; // Static offset of sensor
    const ModelT*               model; // Scanner Model
    const Transform*            Tbm; // Poses
    uint32_t                    Nposes;
    // Handle
    OptixTraversableHandle      handle;
    // Generic Options
    bool                        computeHits;
    bool                        computeRanges;
    bool                        computePoints;
    bool                        computeNormals;
    bool                        computeFaceIds;
    bool                        computeObjectIds;
    // Result
    uint8_t*                    hits;
    float*                      ranges;
    Point*                      points;
    Vector*                     normals;
    unsigned int*               face_ids;
    unsigned int*               object_ids;
};

using OptixSimulationDataGenericSphere = OptixSimulationDataGeneric<SphericalModel>;
using OptixSimulationDataGenericPinhole = OptixSimulationDataGeneric<PinholeModel>;
using OptixSimulationDataGenericO1Dn = OptixSimulationDataGeneric<O1DnModel<VRAM_CUDA> >;


} // namespace imagine

#endif // IMAGINE_SIMULATION_OPTIX_SIMULATION_DATA_HPP