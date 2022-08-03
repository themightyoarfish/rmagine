#ifndef RMAGINE_MAP_OPTIX_MESH_HPP
#define RMAGINE_MAP_OPTIX_MESH_HPP

#include <optix.h>
#include <optix_types.h>
#include <cuda_runtime.h>

#include <rmagine/types/MemoryCuda.hpp>
#include <rmagine/math/types.h>
#include <rmagine/types/mesh_types.h>

#include <rmagine/util/cuda/CudaContext.hpp>
#include <rmagine/util/optix/OptixContext.hpp>

#include <memory>

#include <assimp/mesh.h>

#include "OptixGeometry.hpp"

namespace rmagine
{

/**
//  * @brief Single mesh. 
//  * - Cuda Buffers for vertices, faces, vertex_normals and face_normals
//  * - TraversableHandle for raytracing
//  */
class OptixMesh : public OptixGeometry
{
public:
    using Base = OptixGeometry;

    OptixMesh(OptixContextPtr context = optix_default_context());

    virtual ~OptixMesh();

    virtual void apply();
    virtual void commit();

    Memory<Point, VRAM_CUDA>    vertices;
    Memory<Face, VRAM_CUDA>     faces;
    Memory<Vector, VRAM_CUDA>   face_normals;
    Memory<Vector, VRAM_CUDA>   vertex_normals;

private:
    // after transform
    Memory<Point, VRAM_CUDA>    vertices_;
    Memory<Vector, VRAM_CUDA>   face_normals_;
    Memory<Vector, VRAM_CUDA>   vertex_normals_;
};

using OptixMeshPtr = std::shared_ptr<OptixMesh>;

OptixMeshPtr make_optix_mesh(const aiMesh* amesh);

} // namespace rmagine

#endif // RMAGINE_MAP_OPTIX_MESH_HPP