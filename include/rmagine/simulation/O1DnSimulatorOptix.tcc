#include <rmagine/simulation/optix/O1DnProgramGeneric.hpp>
#include <rmagine/util/optix/OptixDebug.hpp>
#include <optix.h>
#include <optix_stubs.h>

// #include <rmagine/util/StopWatch.hpp>

namespace rmagine
{

template<typename BundleT>
void O1DnSimulatorOptix::preBuildProgram()
{
    if(!m_map)
    {
        throw std::runtime_error("[O1DnSimulatorOptix] preBuildProgram(): No Map available!");
    }

    OptixSimulationDataGenericO1Dn flags;
    setGenericFlags<BundleT>(flags);
    auto it = m_generic_programs.find(flags);
    
    if(it == m_generic_programs.end())
    {
        m_generic_programs[flags] = std::make_shared<O1DnProgramGeneric>(m_map, flags);
    }
}

template<typename BundleT>
void O1DnSimulatorOptix::simulate(
    const Memory<Transform, VRAM_CUDA>& Tbm,
    BundleT& res)
{
    if(!m_map)
    {
        throw std::runtime_error("[O1DnSimulatorOptix] simulate(): No Map available!");
    }
    
    Memory<OptixSimulationDataGenericO1Dn, RAM> mem(1);
    setGenericFlags(res, mem[0]);

    auto it = m_generic_programs.find(mem[0]);
    OptixProgramPtr program;
    if(it == m_generic_programs.end())
    {
        program = std::make_shared<O1DnProgramGeneric>(m_map, mem[0]);
        m_generic_programs[mem[0]] = program;
    } else {
        program = it->second;
        program->updateSBT();
    }

    // set general data

    Memory<O1DnModel_<VRAM_CUDA>, VRAM_CUDA> model(1);
    copy(m_model, model, m_stream);

    mem->Tsb = m_Tsb.raw();
    mem->model = model.raw();
    mem->Tbm = Tbm.raw();
    mem->handle = m_map->scene()->as()->handle;

    // set generic data
    setGenericData(res, mem[0]);

    // 10000 velodynes 
    // - upload Params: 0.000602865s
    // - launch: 5.9642e-05s
    // => this takes too long. Can we somehow preupload stuff?
    Memory<OptixSimulationDataGenericO1Dn, VRAM_CUDA> d_mem(1);
    copy(mem, d_mem, m_stream->handle());

    if(program)
    {
        OPTIX_CHECK( optixLaunch(
                program->pipeline,
                m_stream->handle(),
                reinterpret_cast<CUdeviceptr>(d_mem.raw()), 
                sizeof( OptixSimulationDataGenericO1Dn ),
                &program->sbt,
                m_width, // width Xdim
                m_height, // height Ydim
                Tbm.size() // depth Zdim
                ));
    } else {
        throw std::runtime_error("Return Bundle Combination not implemented for Optix Simulator");
    }

}

template<typename BundleT>
BundleT O1DnSimulatorOptix::simulate(
    const Memory<Transform, VRAM_CUDA>& Tbm)
{
    BundleT res;
    resizeMemoryBundle<VRAM_CUDA>(res, m_width, m_height, Tbm.size());
    simulate(Tbm, res);
    return res;
}

} // namespace rmagine