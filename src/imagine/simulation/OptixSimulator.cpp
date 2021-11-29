#include "imagine/simulation/OptixSimulator.hpp"

#include "imagine/simulation/optix/OptixSimulationData.hpp"
#include "imagine/util/optix/OptixDebug.hpp"

#include <optix.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>

// Scan Programs
#include <imagine/simulation/optix/ScanProgramRanges.hpp>
#include <imagine/simulation/optix/ScanProgramNormals.hpp>
#include <imagine/simulation/optix/ScanProgramGeneric.hpp>

namespace imagine
{

OptixSimulator::OptixSimulator(OptixMapPtr map)
:m_map(map)
{
    m_programs.resize(3);
    
    // programs[0].reset(new ScanProgramHit(mesh));
    m_programs[0].reset(new ScanProgramRanges(map));
    m_programs[1].reset(new ScanProgramNormals(map));
    m_programs[2].reset(new ScanProgramGeneric(map));
    // programs[2].reset(new ScanProgramScanPoint(mesh));
    // programs[3].reset(new ScanProgramFaceId(mesh));
    // programs[4].reset(new ScanProgramObjectId(mesh));
    // programs[5].reset(new ScanProgramPoint(mesh));
    // programs[6].reset(new ScanProgramNormal(mesh));
    // programs[7].reset(new ScanProgramRangeNormal(mesh));

    CUDA_CHECK( cudaStreamCreate( &m_stream ) );
}

OptixSimulator::~OptixSimulator()
{
    cudaStreamDestroy(m_stream);
}

void OptixSimulator::setTsb(const Memory<Transform, RAM>& Tsb)
{
    m_Tsb = Tsb;
}
    
void OptixSimulator::setModel(const Memory<LiDARModel, RAM>& model)
{
    m_width = model->theta.size;
    m_height = model->phi.size;
    m_model = model;
}

void OptixSimulator::simulateRanges(
    const Memory<Transform, VRAM_CUDA>& Tbm, 
    Memory<float, VRAM_CUDA>& ranges) const
{
    Memory<OptixSimulationDataRanges, RAM> mem;
    mem->Tsb = m_Tsb.raw();
    mem->model = m_model.raw();
    mem->Tbm = Tbm.raw();
    mem->handle = m_map->as.handle;
    mem->ranges = ranges.raw();

    Memory<OptixSimulationDataRanges, VRAM_CUDA> d_mem;
    copy(mem, d_mem, m_stream);

    OptixProgramPtr program = m_programs[0];

    if(program)
    {
        OPTIX_CHECK( optixLaunch(
                program->pipeline,
                m_stream,
                reinterpret_cast<CUdeviceptr>(d_mem.raw()), 
                sizeof( OptixSimulationDataRanges ),
                &program->sbt,
                m_width, // width Xdim
                m_height, // height Ydim
                Tbm.size() // depth Zdim
                ));
    } else {
        throw std::runtime_error("Return Bundle Combination not implemented for Optix Simulator");
    }
}

Memory<float, VRAM_CUDA> OptixSimulator::simulateRanges(
    const Memory<Transform, VRAM_CUDA>& Tbm) const
{
    Memory<float, VRAM_CUDA> res(m_width * m_height * Tbm.size());
    simulateRanges(Tbm, res);
    return res;
}

void OptixSimulator::simulateNormals(
    const Memory<Transform, VRAM_CUDA>& Tbm, 
    Memory<Vector, VRAM_CUDA>& normals) const
{
    Memory<OptixSimulationDataNormals, RAM> mem;
    mem->Tsb = m_Tsb.raw();
    mem->model = m_model.raw();
    mem->Tbm = Tbm.raw();
    mem->handle = m_map->as.handle;
    mem->normals = normals.raw();

    Memory<OptixSimulationDataNormals, VRAM_CUDA> d_mem;
    copy(mem, d_mem, m_stream);

    OptixProgramPtr program = m_programs[1];

    if(program)
    {
        OPTIX_CHECK( optixLaunch(
                program->pipeline,
                m_stream,
                reinterpret_cast<CUdeviceptr>(d_mem.raw()), 
                sizeof( OptixSimulationDataNormals ),
                &program->sbt,
                m_width, // width Xdim
                m_height, // height Ydim
                Tbm.size() // depth Zdim
                ));
    } else {
        throw std::runtime_error("Return Bundle Combination not implemented for Optix Simulator");
    }
}

Memory<Vector, VRAM_CUDA> OptixSimulator::simulateNormals(
    const Memory<Transform, VRAM_CUDA>& Tbm) const
{
    Memory<Vector, VRAM_CUDA> res(m_width * m_height * Tbm.size());
    simulateNormals(Tbm, res);
    return res;
}

void OptixSimulator::simulate(
    const Memory<Transform, VRAM_CUDA>& Tbm,
    Memory<float, VRAM_CUDA>& ranges,
    Memory<Vector, VRAM_CUDA>& normals) const
{
    Memory<OptixSimulationDataGeneric, RAM> mem;
    mem->Tsb = m_Tsb.raw();
    mem->model = m_model.raw();
    mem->Tbm = Tbm.raw();
    mem->handle = m_map->as.handle;
    mem->ranges = ranges.raw();
    mem->normals = normals.raw();

    Memory<OptixSimulationDataGeneric, VRAM_CUDA> d_mem;
    copy(mem, d_mem, m_stream);

    OptixProgramPtr program = m_programs[2];

    if(program)
    {
        OPTIX_CHECK( optixLaunch(
                program->pipeline,
                m_stream,
                reinterpret_cast<CUdeviceptr>(d_mem.raw()), 
                sizeof( OptixSimulationDataGeneric ),
                &program->sbt,
                m_width, // width Xdim
                m_height, // height Ydim
                Tbm.size() // depth Zdim
                ));
    } else {
        throw std::runtime_error("Return Bundle Combination not implemented for Optix Simulator");
    }

}

} // imagine