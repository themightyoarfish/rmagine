#include "rmagine/simulation/PinholeSimulatorOptix.hpp"

#include "rmagine/simulation/optix/sim_program_data.h"
#include "rmagine/util/optix/OptixDebug.hpp"

#include <optix.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>

// Scan Programs
// #include <rmagine/simulation/optix/PinholeProgramRanges.hpp>
// #include <rmagine/simulation/optix/PinholeProgramNormals.hpp>

namespace rmagine
{

PinholeSimulatorOptix::PinholeSimulatorOptix()
:m_model(1)
,m_Tsb(1)
{
    Memory<Transform, RAM_CUDA> I(1);
    I->setIdentity();
    m_Tsb = I;
}

PinholeSimulatorOptix::PinholeSimulatorOptix(OptixMapPtr map)
:PinholeSimulatorOptix()
{
    setMap(map);
}

PinholeSimulatorOptix::~PinholeSimulatorOptix()
{
    // m_programs.resize(0);
}

void PinholeSimulatorOptix::setMap(const OptixMapPtr map)
{
    m_map = map;
    // none generic version
    // m_programs.resize(2);
    // m_programs[0] = std::make_shared<PinholeProgramRanges>(map);
    // m_programs[1] = std::make_shared<PinholeProgramNormals>(map);

    // need to create stream after map was created: cuda device api context is required
    m_stream = m_map->stream();
}

void PinholeSimulatorOptix::setTsb(const Memory<Transform, RAM>& Tsb)
{
    m_Tsb = Tsb;
}

void PinholeSimulatorOptix::setTsb(const Transform& Tsb)
{
    Memory<Transform, RAM> tmp(1);
    tmp[0] = Tsb;
    setTsb(tmp);
}
    
void PinholeSimulatorOptix::setModel(const Memory<PinholeModel, RAM>& model)
{
    m_width = model->width;
    m_height = model->height;
    m_model = model;

    Memory<SensorModelUnion, RAM> model_union(1);
    model_union->pinhole = m_model.raw();
    m_model_union = model_union;
}

void PinholeSimulatorOptix::setModel(const PinholeModel& model)
{
    Memory<PinholeModel, RAM> tmp(1);
    tmp[0] = model;
    setModel(tmp);
}

void PinholeSimulatorOptix::simulateRanges(
    const Memory<Transform, VRAM_CUDA>& Tbm, 
    Memory<float, VRAM_CUDA>& ranges) const
{
    if(!m_map)
    {
        // no map set
        throw std::runtime_error("[PinholeSimulatorOptix] simulateRanges(): No Map available!");
        return;
    }

    auto optix_ctx = m_map->context();
    auto cuda_ctx = optix_ctx->getCudaContext();
    if(!cuda_ctx->isActive())
    {
        std::cout << "[SphereSimulatorOptix::simulateRanges() Need to activate map context" << std::endl;
        cuda_ctx->use();
    }

    Memory<OptixSimulationDataRangesPinhole, RAM> mem(1);
    mem->Tsb = m_Tsb.raw();
    mem->model = m_model.raw();
    mem->Tbm = Tbm.raw();
    mem->handle = m_map->scene()->as()->handle;
    mem->ranges = ranges.raw();

    Memory<OptixSimulationDataRangesPinhole, VRAM_CUDA> d_mem(1);
    copy(mem, d_mem, m_stream->handle());

    PipelinePtr program;// = m_programs[0];

    // if(program)
    // {
    //     program->updateSBT();
    //     RM_OPTIX_CHECK( optixLaunch(
    //             program->pipeline,
    //             m_stream->handle(),
    //             reinterpret_cast<CUdeviceptr>(d_mem.raw()), 
    //             sizeof( OptixSimulationDataRangesPinhole ),
    //             &program->sbt,
    //             m_width, // width Xdim
    //             m_height, // height Ydim
    //             Tbm.size() // depth Zdim
    //             ));
    // } else {
    //     throw std::runtime_error("Return Bundle Combination not implemented for Optix Simulator");
    // }
}

Memory<float, VRAM_CUDA> PinholeSimulatorOptix::simulateRanges(
    const Memory<Transform, VRAM_CUDA>& Tbm) const
{
    Memory<float, VRAM_CUDA> res(m_width * m_height * Tbm.size());
    simulateRanges(Tbm, res);
    return res;
}

void PinholeSimulatorOptix::launch(
    const Memory<OptixSimulationDataGeneric, RAM>& mem,
    PipelinePtr program)
{
    Memory<OptixSimulationDataGeneric, VRAM_CUDA> d_mem(1);
    copy(mem, d_mem, m_stream->handle());

    RM_OPTIX_CHECK( optixLaunch(
                program->pipeline,
                m_stream->handle(),
                reinterpret_cast<CUdeviceptr>(d_mem.raw()), 
                sizeof( OptixSimulationDataGeneric ),
                program->sbt,
                m_width, // width Xdim
                m_height, // height Ydim
                mem->Nposes // depth Zdim
                ) );
}

} // rmagine