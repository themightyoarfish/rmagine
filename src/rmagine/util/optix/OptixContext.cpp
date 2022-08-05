#include "rmagine/util/optix/OptixContext.hpp"
#include "rmagine/util/optix/OptixDebug.hpp"
#include <optix.h>
#include <optix_stubs.h>
#include <iomanip>

static void context_log_cb( unsigned int level, const char* tag, const char* message, void* /*cbdata */)
{
    std::cerr << "[" << std::setw( 2 ) << level << "][" << std::setw( 12 ) << tag << "]: "
              << message << "\n";
}

namespace rmagine {

bool optix_initialized_ = false;

bool optix_initialized()
{
    return optix_initialized_;
}

void optix_initialize()
{
    std::stringstream optix_version_str;
    optix_version_str << OPTIX_VERSION / 10000 << "." << (OPTIX_VERSION % 10000) / 100 << "." << OPTIX_VERSION % 100;
    std::cout << "[OptixContext] Init Optix (" << optix_version_str.str() << ") context on latest CUDA context " << std::endl;
    OPTIX_CHECK( optixInit() );
    optix_initialized_ = true;
}

OptixContext::OptixContext(CudaContextPtr cuda_context)
:m_cuda_context(cuda_context)
{
    init(cuda_context);
    std::cout << "[OptixContext::OptixContext()] constructed." << std::endl;
}

OptixContext::~OptixContext()
{
    optixDeviceContextDestroy( m_optix_context );
    std::cout << "[OptixContext::~OptixContext()] destroyed." << std::endl;
}

CudaContextPtr OptixContext::getCudaContext()
{
    return m_cuda_context;
}

OptixDeviceContext OptixContext::ref()
{
    return m_optix_context;
}

void OptixContext::init(CudaContextPtr cuda_context)
{
    if(!optix_initialized())
    {
        optix_initialize();
    }

    // Specify context options
    OptixDeviceContextOptions options = {};
    options.logCallbackFunction       = &context_log_cb;
    options.logCallbackLevel          = 0;

    OPTIX_CHECK( optixDeviceContextCreate( cuda_context->ref(), &options, &m_optix_context ) );
}

OptixContextPtr optix_def_ctx(new OptixContext(cuda_current_context()) );

OptixContextPtr optix_default_context()
{   
    return optix_def_ctx;
}

} // namespace rmagine