#include <iostream>
#include <sstream>

// General rmagine includes
#include <rmagine/types/sensor_models.h>
#include <rmagine/util/StopWatch.hpp>
#include <rmagine/util/synthetic.h>

// #include <rmagine/map/OptixMap.hpp>
#include <rmagine/map/optix/OptixMesh.hpp>
#include <rmagine/map/optix/OptixBoxes.hpp>
#include <rmagine/map/optix/OptixInst.hpp>
#include <rmagine/map/optix/OptixInstances.hpp>
#include <rmagine/map/optix/OptixScene.hpp>

#include <rmagine/util/prints.h>
#include <rmagine/util/StopWatch.hpp>

#include <rmagine/util/IDGen.hpp>

#include <rmagine/simulation/SphereSimulatorOptix.hpp>
#include <rmagine/map/optix/optix_shapes.h>

using namespace rmagine;
namespace rm = rmagine;

SphericalModel single_ray_model()
{
    SphericalModel model;
    model.theta.min = 0.0;
    model.theta.inc = 0.0;
    model.theta.size = 1;

    model.phi.min = 0.0;
    model.phi.inc = 1.0;
    model.phi.size = 1;
    
    model.range.min = 0.0;
    model.range.max = 130.0;
    return model;
}

SphereSimulatorOptixPtr make_sim(OptixScenePtr scene)
{
    SphereSimulatorOptixPtr gpu_sim = std::make_shared<SphereSimulatorOptix>(scene);
    SphericalModel model = single_ray_model();
    gpu_sim->setModel(model);
    return gpu_sim;
}

void printRaycast(SphereSimulatorOptixPtr gpu_sim,
    Vector3 pos, 
    EulerAngles angles)
{
    Transform T = Transform::Identity();
    T.t = pos;
    T.R.set(angles);
    
    Memory<Transform, RAM> Tbm(1);
    Tbm[0] = T;

    Memory<Transform, VRAM_CUDA> Tbm_gpu;
    Tbm_gpu = Tbm;

    // std::cout << "Simulate!" << std::endl;

    using ResultT = Bundle<
        Ranges<VRAM_CUDA>,
        Normals<VRAM_CUDA>,
        FaceIds<VRAM_CUDA>,
        ObjectIds<VRAM_CUDA>
    >;
    ResultT res = gpu_sim->simulate<ResultT>(Tbm_gpu);

    // Download results
    Memory<float, RAM> ranges = res.ranges;
    Memory<Vector, RAM> normals = res.normals;
    Memory<unsigned int, RAM> face_ids = res.face_ids;
    Memory<unsigned int, RAM> obj_ids = res.object_ids;

    // print results
    std::cout << "Result:" << std::endl;
    std::cout << "- range: " << ranges[0] << std::endl;
    std::cout << "- normal: " << normals[0] << std::endl;
    std::cout << "- face id: " << face_ids[0] << std::endl;
    std::cout << "- obj id: " << obj_ids[0] << std::endl;
}

void printRaycast(
    OptixScenePtr scene, 
    Vector3 pos, 
    EulerAngles angles)
{
    auto gpu_sim = make_sim(scene);
    printRaycast(gpu_sim, pos, angles);
}

OptixMeshPtr custom_mesh()
{
    OptixMeshPtr mesh = std::make_shared<OptixMesh>();

    Memory<Point, RAM> vertices_cpu(3);
    vertices_cpu[0] = {0.0, 0.5, 0.5};
    vertices_cpu[1] = {0.0, 0.5, -0.5};
    vertices_cpu[2] = {0.0, -0.5, -0.5};
    mesh->vertices = vertices_cpu;

    Memory<Face, RAM> faces_cpu(1);
    faces_cpu[0] = {0, 1, 2};
    mesh->faces = faces_cpu;

    mesh->computeFaceNormals();

    Memory<Vector, RAM> normals = mesh->face_normals;
    std::cout << "Computed normal: " << normals[0] << std::endl;

    Transform T;
    T.setIdentity();
    mesh->setTransform(T);
    mesh->apply();

    return mesh;
}

void scene_1()
{
    std::cout << "Make Optix Mesh" << std::endl;

    OptixScenePtr scene = std::make_shared<OptixScene>(); 

    OptixGeometryPtr geom1 = custom_mesh();
    geom1->commit();
    OptixGeometryPtr geom2 = std::make_shared<OptixSphere>(50, 50);
    geom2->commit();

    scene->add(geom1);
    scene->add(geom2);

    OptixInstancesPtr insts = std::make_shared<OptixInstances>();

    {   // two custom instances (5, 0, 0) and (5, 5, 0)
        OptixInstPtr geom_inst_1 = std::make_shared<OptixInst>();
        geom_inst_1->setGeometry(geom1);

        Transform T = Transform::Identity();
        T.t.x = 5.0;
        geom_inst_1->setTransform(T);
        geom_inst_1->apply();
        insts->add(geom_inst_1);
        std::cout << T << std::endl;

        OptixInstPtr geom_inst_2 = std::make_shared<OptixInst>();
        geom_inst_2->setGeometry(geom1);
        T.t.x = 10.0;
        geom_inst_2->setTransform(T);
        geom_inst_2->apply();
        insts->add(geom_inst_2);
    }

    { // 10 sphere instances at z = 10 from x=0 to x=10
        for(size_t i=0; i<10; i++)
        {
            OptixInstPtr geom_inst = std::make_shared<OptixInst>();

            geom_inst->setGeometry(geom2);

            Transform T = Transform::Identity();
            T.t.z = 10.0;
            T.t.x = static_cast<float>(i);
            geom_inst->setTransform(T);
            geom_inst->apply();

            unsigned int id = insts->add(geom_inst);
            std::cout << "Created Sphere instance " << id << std::endl;
        }
    }

    std::cout << "Commit Instances" << std::endl;
    insts->commit();

    scene->setRoot(insts);
    scene->commit();
    std::cout << "Scene committed" << std::endl;

    printRaycast(scene, {0.0, 0.0, -0.1}, {0.0, 0.0, 0.0});
    printRaycast(scene, {0.0, 0.0, 10.0}, {0.0, 0.0, 0.0});
}

void scene_2()
{
    OptixScenePtr scene = std::make_shared<OptixScene>();

    OptixGeometryPtr geom = std::make_shared<OptixCube>();
    geom->name = "Cube";
    geom->commit();
    scene->add(geom);

    OptixInstancesPtr insts = std::make_shared<OptixInstances>();

    for(size_t i=0; i<10; i++)
    {
        OptixInstPtr inst_geom = std::make_shared<OptixInst>();
        inst_geom->setGeometry(geom);
        inst_geom->name = "Instance " + i;
        Transform T = Transform::Identity();
        T.t.y = static_cast<float>(i);
        T.t.x = 10.0;
        inst_geom->setTransform(T);
        inst_geom->apply();
        insts->add(inst_geom);
    }
    insts->commit();

    scene->setRoot(insts);

    printRaycast(scene, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});

    std::cout << "UPDATE SCENE" << std::endl;

    {
        // move instance 5 behind instance 0
        OptixInstPtr inst = insts->get(5);
        Transform T = inst->transform();
        T.t.y = 0.0;
        T.t.x = 15.0;
        inst->setTransform(T);
        inst->apply();

        // disable istance 0
        insts->get(0)->disable();
    }

    insts->commit();

    printRaycast(scene, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
}

void scene_3()
{
    OptixScenePtr scene = std::make_shared<OptixScene>();

    OptixGeometryPtr geom = std::make_shared<OptixCube>();
    geom->name = "Cube";
    geom->commit();
    scene->add(geom);

    OptixInstancesPtr insts = std::make_shared<OptixInstances>();

    for(size_t i=0; i<10; i++)
    {
        OptixInstPtr inst_geom = std::make_shared<OptixInst>();
        inst_geom->setGeometry(geom);
        inst_geom->name = "Instance " + i;
        Transform T = Transform::Identity();
        T.t.y = static_cast<float>(i);
        T.t.x = 10.0;
        inst_geom->setTransform(T);
        inst_geom->apply();
        insts->add(inst_geom);
    }
    insts->commit();

    scene->setRoot(insts);
    scene->commit();

    printRaycast(scene, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});

    // add in front of 0
    OptixInstPtr inst_geom = std::make_shared<OptixInst>();
    {
        inst_geom->setGeometry(geom);
        Transform T = Transform::Identity();
        T.t.x = 5.0;
        inst_geom->setTransform(T);
        inst_geom->apply();
    }
    unsigned int inst_id;
    inst_id = insts->add(inst_geom);
    insts->commit();
    scene->commit();
    std::cout << "Added Instance with id " << inst_id << std::endl;

    printRaycast(scene, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});

    // remove
    insts->remove(inst_geom);
    insts->commit();
    scene->commit();
    std::cout << "Removed Instance with id " << inst_id << std::endl;

    printRaycast(scene, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});

    
    inst_id = insts->add(inst_geom);
    insts->commit();
    scene->commit();
    std::cout << "Added Instance with id " << inst_id << std::endl;

    printRaycast(scene, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});

}

void scene_4()
{
    // sensor update problem

    OptixScenePtr scene = std::make_shared<OptixScene>();

    OptixGeometryPtr geom = std::make_shared<OptixCube>();
    geom->name = "Cube";
    geom->commit();
    scene->add(geom);

    OptixInstancesPtr insts = std::make_shared<OptixInstances>();

    for(size_t i=0; i<2; i++)
    {
        OptixInstPtr inst_geom = std::make_shared<OptixInst>();
        inst_geom->setGeometry(geom);
        inst_geom->name = "Instance " + i;
        Transform T = Transform::Identity();
        T.t.x = static_cast<float>(i) * 5.0 + 5.0;
        inst_geom->setTransform(T);
        inst_geom->apply();
        insts->add(inst_geom);
    }
    insts->commit();

    scene->setRoot(insts);
    scene->commit();

    auto sim = make_sim(scene);
    printRaycast(sim, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});

    OptixInstPtr inst_geom = std::make_shared<OptixInst>();
    {
        inst_geom->setGeometry(geom);
        inst_geom->name = "Instance NEW";
        Transform T = Transform::Identity();
        T.t.x = 1.0;
        inst_geom->setTransform(T);
        inst_geom->apply();
        insts->add(inst_geom);
    }
    insts->commit();
    scene->commit();

    printRaycast(sim, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0});
}

int main(int argc, char** argv)
{
    std::cout << "Rmagine Optix Scene Building" << std::endl;

    int example = 1;

    if(argc > 1)
    {
        example = std::stoi( argv[1] );
    }

    std::cout << "SCENE EXAMPLE " << example << std::endl;
    
    switch(example)
    {
        case 1: scene_1(); break;
        case 2: scene_2(); break;
        case 3: scene_3(); break;
        case 4: scene_4(); break;
        default: break;
    }

    return 0;
}