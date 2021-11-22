#ifndef IMAGINE_EMBREE_SIMULATOR_HPP
#define IMAGINE_EMBREE_SIMULATOR_HPP

#include <imagine/map/EmbreeMap.hpp>
#include <imagine/types/Memory.hpp>
#include <imagine/types/sensor_models.h>
#include "SimulationResults.hpp"

namespace imagine
{

class EmbreeSimulator {
public:
    EmbreeSimulator(const EmbreeMapPtr mesh);
    ~EmbreeSimulator();

    using MEM = RAM;

    void setTsb(const Memory<Transform, RAM>& Tsb);
    void setModel(const Memory<LiDARModel, RAM>& model);

    void simulateRanges(
        const Memory<Transform, RAM>& Tbm, 
        Memory<float, RAM>& ranges);

    Memory<float, RAM> simulateRanges(
        const Memory<Transform, RAM>& Tbm);

    void simulateHits(
        const Memory<Transform, RAM>& Tbm, 
        Memory<uint8_t, RAM>& hits);

    Memory<uint8_t, RAM> simulateHits(
        const Memory<Transform, RAM>& Tbm);



    // Generic Version
    template<typename BundleT>
    void simulate(const Memory<Transform, RAM>& Tbm,
        BundleT& ret);

    template<typename BundleT>
    BundleT simulate(const Memory<Transform, RAM>& Tbm);
    
protected:
    const EmbreeMapPtr m_map;
    RTCIntersectContext m_context;

    Memory<Transform, RAM> m_Tsb;
    Memory<LiDARModel, RAM> m_model;
};

using EmbreeSimulatorPtr = std::shared_ptr<EmbreeSimulator>;

} // namespace imagine

#include "EmbreeSimulator.tcc"

#endif // IMAGINE_EMBREE_SIMULATOR_HPP