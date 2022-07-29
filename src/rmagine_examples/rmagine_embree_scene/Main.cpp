#include <iostream>
#include <sstream>

// General rmagine includes
#include <rmagine/types/sensor_models.h>
#include <rmagine/util/StopWatch.hpp>
#include <rmagine/util/synthetic.h>

#include <rmagine/map/EmbreeMap.hpp>

#include <rmagine/map/embree/embree_shapes.h>
#include <rmagine/util/prints.h>
#include <rmagine/util/StopWatch.hpp>

using namespace rmagine;

void scene_1()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();
    
    {
        // add sphere. sphere should stay alive as long as the scene lives
        EmbreeMeshPtr sphere = std::make_shared<EmbreeSphere>(1.0);
        std::cout << sphere->scale() << std::endl;
        scene->add(sphere);
    }
    std::cout << "now sphere should be alive" << std::endl;

    scene.reset();
    std::cout << "Scene destroyed." << std::endl;
}

void scene_2()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();

    unsigned int sphere_id = 0;
    {
        // add sphere. sphere should stay alive as long as the scene lives
        EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
        std::cout << sphere->scale() << std::endl;
        sphere_id = scene->add(sphere);
    }

    std::cout << "now sphere should be alive" << std::endl;

    // save mesh
    EmbreeGeometryPtr sphere = scene->geometries()[sphere_id];

    scene.reset();
    std::cout << "Now Scene should be destroyed but not the sphere" << std::endl;

    if(!sphere->parent.lock())
    {
        std::cout << "- sphere noticed parent was destroyed." << std::endl;
    }
}

void scene_3()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();
    unsigned int sphere_id = 0;
    {
        // add sphere. sphere should stay alive as long as the scene lives
        EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
        std::cout << sphere->scale() << std::endl;
        sphere_id = scene->add(sphere);
    }

    EmbreeGeometryPtr sphere = scene->remove(sphere_id);
    

    std::cout << "Removed mesh from scene. the next numbers should be 0" << std::endl;
    std::cout << scene->count<EmbreeMesh>() << std::endl;
    std::cout << sphere->parent.lock() << std::endl;
}

void scene_4()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();
    
    
    // add sphere. sphere should stay alive as long as the scene lives
    EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
    std::cout << sphere->scale() << std::endl;
    scene->add(sphere);
    

    // the scenes mesh should get invalid when destroying mesh
    // sphere.reset();
    std::cout << "meshes: " << scene->count<EmbreeMesh>() << std::endl;

    sphere->commit();
    scene->commit();

    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    RTCRayHit rayhit;
    rayhit.ray.org_x = 0;
    rayhit.ray.org_y = 0;
    rayhit.ray.org_z = 0;
    rayhit.ray.dir_x = 1;
    rayhit.ray.dir_y = 0;
    rayhit.ray.dir_z = 0;
    rayhit.ray.tnear = 0;
    rayhit.ray.tfar = INFINITY;
    rayhit.ray.mask = 0;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(scene->handle(), &context, &rayhit);

    std::cout << "Raycast:" << std::endl;

    std::cout << "- range: " << rayhit.ray.tfar << std::endl;

    if(rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        
        std::cout << "- geomID: " << rayhit.hit.geomID << std::endl;
    }

    if(rayhit.hit.instID[0] != RTC_INVALID_GEOMETRY_ID)
    {
        std::cout << "- instID: " << rayhit.hit.instID[0] << std::endl;
    }
    
}

void printRaycast(EmbreeScenePtr scene, Vector3 orig, Vector3 dir)
{
    RTCIntersectContext context;
    rtcInitIntersectContext(&context);

    RTCRayHit rayhit;
    rayhit.ray.org_x = orig.x;
    rayhit.ray.org_y = orig.y;
    rayhit.ray.org_z = orig.z;
    rayhit.ray.dir_x = dir.x;
    rayhit.ray.dir_y = dir.y;
    rayhit.ray.dir_z = dir.z;
    rayhit.ray.tnear = 0;
    rayhit.ray.tfar = INFINITY;
    rayhit.ray.mask = 0;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(scene->handle(), &context, &rayhit);

    std::cout << "Raycast:" << std::endl;

    std::cout << "- range: " << rayhit.ray.tfar << std::endl;

    if(rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        if(rayhit.hit.instID[0] != RTC_INVALID_GEOMETRY_ID)
        {
            auto geom = scene->get(rayhit.hit.instID[0]);
            std::cout << "- id: " << rayhit.hit.instID[0] << std::endl;
            std::cout << "- type: instance" << std::endl;
            std::cout << "- name: " << geom->name << std::endl;
        } else {
            auto geom = scene->get(rayhit.hit.geomID);
            std::cout << "- id: " << rayhit.hit.geomID << std::endl;
            std::cout << "- type: mesh" << std::endl;
            std::cout << "- name: " << geom->name << std::endl;
        }
        
    }

    if(rayhit.hit.instID[0] != RTC_INVALID_GEOMETRY_ID)
    {
        std::cout << "- instID: " << rayhit.hit.instID[0] << std::endl;
    }
}

void scene_5()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();

    {
        std::cout << "Generate Sphere" << std::endl;
        EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
        Transform T;
        T.setIdentity();
        T.t.y = 5.0;

        sphere->setTransform(T);
        sphere->apply();
        sphere->commit();

        scene->add(sphere);
    }

    {
        std::cout << "Generate Cube" << std::endl;
        EmbreeCubePtr cube = std::make_shared<EmbreeCube>();
        Transform T;
        T.setIdentity();
        T.t.y = -5.0;
        cube->setTransform(T);

        cube->apply();
        cube->commit();

        scene->add(cube);
    }

    std::cout << "Commit Scene" << std::endl;
    scene->commit();

    std::cout << "Raycast.." << std::endl;
    printRaycast(scene, {0.0, 5.0, 0.0}, {1.0, 0.0, 0.0});
    printRaycast(scene, {0.0, -5.0, 0.0}, {1.0, 0.0, 0.0});
}

void scene_6()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();


    { // CUBE INSTANCE
        EmbreeCubePtr cube = std::make_shared<EmbreeCube>();
        cube->commit();

        // make cube scene
        EmbreeScenePtr cube_scene = std::make_shared<EmbreeScene>();
        cube_scene->add(cube);
        cube_scene->commit();

        // make cube scene instance
        EmbreeInstancePtr cube_inst = std::make_shared<EmbreeInstance>();
        cube_inst->set(cube_scene);

        Transform T;
        T.setIdentity();
        T.t.y = -5.0;
        Matrix4x4 M;
        M.set(T);

        cube_inst->setTransform(M);
        cube_inst->apply();

        cube_inst->commit();

        scene->add(cube_inst);
    }

    { // SPHERE MESH
        std::cout << "Generate Sphere" << std::endl;
        EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
        Transform T;
        T.setIdentity();
        T.t.y = 5.0;

        sphere->setTransform(T);
        sphere->apply();
        sphere->commit();

        scene->add(sphere);
    }

    std::cout << "Commit Scene" << std::endl;
    scene->commit();

    std::cout << "Raycast.." << std::endl;
    printRaycast(scene, {0.0, 5.0, 0.0}, {1.0, 0.0, 0.0});
    printRaycast(scene, {0.0, -5.0, 0.0}, {1.0, 0.0, 0.0});
}

void scene_7()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();

    std::cout << "Generate Sphere" << std::endl;
    // gen sphere mesh
    EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
    sphere->commit();

    // make sphere scene
    EmbreeScenePtr sphere_scene = std::make_shared<EmbreeScene>();
    sphere_scene->add(sphere);
    sphere_scene->commit();

    // make N sphere instances
    int Ninstances = 100;
    for(int i=0; i < Ninstances; i++)
    {
        EmbreeInstancePtr sphere_inst = std::make_shared<EmbreeInstance>();
        sphere_inst->set(sphere_scene);

        float t = static_cast<float>(i - Ninstances / 2);

        Transform T;
        T.setIdentity();
        T.t.y = t;

        Vector3 scale = {
            0.01f * static_cast<float>(i + 1),
            0.01f * static_cast<float>(i + 1),
            0.01f * static_cast<float>(i + 1)
        };

        sphere_inst->setTransform(T);
        sphere_inst->setScale(scale);
        sphere_inst->apply();
        sphere_inst->commit();

        scene->add(sphere_inst);
    }

    std::cout << "Commit Scene" << std::endl;
    scene->commit();

    std::cout << "Raycast.." << std::endl;
    printRaycast(scene, {0.0, 5.0, 0.0}, {1.0, 0.0, 0.0});
    printRaycast(scene, {0.0, -5.0, 0.0}, {1.0, 0.0, 0.0});
} 

void scene_8()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();

    std::cout << "Generate Sphere" << std::endl;
    // gen sphere mesh
    EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
    sphere->name = "Sphere Mesh";
    sphere->commit();

    // make sphere scene
    EmbreeScenePtr sphere_scene = std::make_shared<EmbreeScene>();
    sphere_scene->add(sphere);
    sphere_scene->commit();

    // make N sphere instances
    int Ninstances = 100;
    for(int i=0; i < Ninstances; i++)
    {
        EmbreeInstancePtr sphere_inst = std::make_shared<EmbreeInstance>();
        sphere_inst->set(sphere_scene);

        float t = static_cast<float>(i - Ninstances / 2);

        Transform T;
        T.setIdentity();
        T.t.y = t;

        Vector3 scale = {
            0.01f * static_cast<float>(i + 1),
            0.01f * static_cast<float>(i + 1),
            0.01f * static_cast<float>(i + 1)
        };
        
        std::stringstream ss;
        ss << "Sphere Instance " << i;
        sphere_inst->name = ss.str();
        sphere_inst->setTransform(T);
        sphere_inst->setScale(scale);
        sphere_inst->apply();
        sphere_inst->commit();

        scene->add(sphere_inst);
    }

    unsigned int cube_id = 0;
    { // CUBE MESH
        EmbreeCubePtr cube = std::make_shared<EmbreeCube>();
        cube->name = "Cube";

        Transform T;
        T.setIdentity();
        T.t.x += 10.0;
        cube->setTransform(T);
        cube->apply();
        cube->commit();

        cube_id = scene->add(cube);
    }

    std::cout << "Commit Scene" << std::endl;
    scene->commit();

    std::cout << "Raycast.." << std::endl;
    printRaycast(scene, {0.0, 5.0, 0.0}, {1.0, 0.0, 0.0});
    printRaycast(scene, {0.0, -5.0, 0.0}, {1.0, 0.0, 0.0});

    // try to hit cube
    printRaycast(scene, {5.0, 0.0, 0.0}, {1.0, 0.0, 0.0});


    // remove cube
    EmbreeCubePtr cube = std::dynamic_pointer_cast<EmbreeCube>(scene->remove(cube_id));
    scene->commit();

    printRaycast(scene, {5.0, 0.0, 0.0}, {1.0, 0.0, 0.0});

    // add cube again
    scene->add(cube);
    scene->commit();

    printRaycast(scene, {5.0, 0.0, 0.0}, {1.0, 0.0, 0.0});

    // move cube
    
    auto T = cube->transform();
    T.t.x += 0.5;
    cube->setTransform(T);
    cube->apply();
    cube->commit();

    scene->commit();

    printRaycast(scene, {5.0, 0.0, 0.0}, {1.0, 0.0, 0.0});

    // add ground plane
    EmbreePlanePtr plane = std::make_shared<EmbreePlane>();
    plane->name = "Ground Plane";
    plane->setScale({1000.0, 1000.0, 1.0});
    plane->apply();
    plane->commit();

    scene->add(plane);
    scene->commit();

    printRaycast(scene, {-5.0, 0.0, 5.0}, {0.0, 0.0, -1.0});
}

EmbreeScenePtr make_scene()
{
    EmbreeScenePtr scene = std::make_shared<EmbreeScene>();
    scene->setQuality(RTC_BUILD_QUALITY_LOW);
    scene->setFlags(RTC_SCENE_FLAG_DYNAMIC);

    { 
        // gen sphere mesh
        EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>(1.0);
        sphere->name = "Sphere Mesh";
        sphere->commit();

        // make sphere scene
        EmbreeScenePtr sphere_scene = std::make_shared<EmbreeScene>();
        sphere_scene->add(sphere);
        sphere_scene->commit();

        // make N sphere instances
        int Ninstances = 100;
        for(int i=0; i < Ninstances; i++)
        {
            EmbreeInstancePtr sphere_inst = std::make_shared<EmbreeInstance>();
            sphere_inst->set(sphere_scene);

            Transform T;
            T.setIdentity();
            T.t.y = static_cast<float>(i - Ninstances / 2);

            Vector3 scale = {
                0.01f * static_cast<float>(i + 1),
                0.01f * static_cast<float>(i + 1),
                0.01f * static_cast<float>(i + 1)
            };
            
            std::stringstream ss;
            ss << "Sphere Instance " << i;
            sphere_inst->name = ss.str();
            sphere_inst->setTransform(T);
            sphere_inst->setScale(scale);
            sphere_inst->apply();
            sphere_inst->commit();

            scene->add(sphere_inst);
        }
    }

    { // NOT INSTANCES SPHERE
        int Ninstances = 100;
        for(int i=0; i<Ninstances; i++)
        {
            EmbreeSpherePtr sphere = std::make_shared<EmbreeSphere>();
            std::stringstream ss;
            ss << "Sphere Mesh " << i;
            sphere->name = ss.str();

            Transform T;
            T.setIdentity();
            T.t.x = -10.0;
            T.t.y = static_cast<float>(i - (Ninstances / 2) );

            Vector3 scale = {
                0.01f * static_cast<float>(i + 1),
                0.01f * static_cast<float>(i + 1),
                0.01f * static_cast<float>(i + 1)
            };

            sphere->setTransform(T);
            sphere->setScale(scale);
            sphere->apply();
            sphere->commit();

            scene->add(sphere);
        }
        
    }

    { // CUBE MESH
        EmbreeCubePtr cube = std::make_shared<EmbreeCube>();
        cube->name = "Cube";

        Transform T;
        T.setIdentity();
        T.t.x += 10.0;
        T.t.z += 0.5;
        cube->setTransform(T);
        cube->apply();
        cube->commit();

        scene->add(cube);
    }

    return scene;
}

void scene_9()
{
    StopWatch sw;
    double el;


    EmbreeScenePtr scene = make_scene();

    // test scene update runtimes

    // 1. no change update

    sw();
    scene->commit();
    el = sw();
    std::cout << "No changes update in " << el << "s" << std::endl;

    // should hit "Sphere Standalone" here
    // printRaycast(scene, {5.0, 10.0, 0.5}, {1.0, 0.0, 0.0});

    
    // 2. instances update
    {
        
        sw();
        for(size_t i=0; i<100; i++)
        {
            EmbreeInstancePtr inst = scene->get_as<EmbreeInstance>(i);
            auto T = inst->transform();
            T.t.x -= 5.0;
            inst->setTransform(T);
            inst->apply();
            inst->commit();
        }
        double prep_el = sw();

        sw();
        scene->commit();
        el = sw();
        std::cout << "Move " << 100 << " instances and update scene in " << prep_el + el << "s (" << prep_el << " + " << el << ")" << std::endl;

        printRaycast(scene, {-5.0, 49.0, 0.0}, {1.0, 0.0, 0.0});
    }

    // 3. mesh update
    {
        sw();
        for(size_t i=100; i<200; i++)
        {
            EmbreeMeshPtr sphere = scene->get_as<EmbreeMesh>(i);
            auto T = sphere->transform();
            T.t.x -= 5.0;
            sphere->setTransform(T);
            sphere->apply();
            sphere->commit();
        }

        double prep_el = sw();

        sw();
        scene->commit();
        el = sw();
        std::cout << "Move " << 100 << " meshes and update scene in " << prep_el + el << "s (" << prep_el << " + " << el << ")" << std::endl;

        printRaycast(scene, {-15.0, 49.0, 0.0}, {1.0, 0.0, 0.0});
    }
}

void scene_10()
{
    EmbreeScenePtr scene = make_scene();
    EmbreeScenePtr scene2 = make_scene();

    scene->commit();

    EmbreeGeometryPtr inst99 = scene2->get(99);
    auto T = inst99->transform();
    T.t.x += 5.0;
    inst99->setTransform(T);
    inst99->apply();
    inst99->commit();

    scene2->commit();

    printRaycast(scene, {-5.0, 49.0, 0.0}, {1.0, 0.0, 0.0});

    printRaycast(scene2, {-5.0, 49.0, 0.0}, {1.0, 0.0, 0.0});
}

void scene_11()
{
    EmbreeScenePtr scene = make_scene();
    EmbreeScenePtr scene2 = std::make_shared<EmbreeScene>();


    EmbreeCylinderPtr cylinder = std::make_shared<EmbreeCylinder>();
    
    Transform T;
    T.setIdentity();
    T.t.z = 50.0;
    cylinder->setTransform(T);
    cylinder->apply();
    cylinder->commit();
    cylinder->name = "MyCylinder";
    scene2->add(cylinder);
    scene2->commit();
    
    printRaycast(scene2, {0.0, 0.0, 50.0}, {1.0, 0.0, 0.0});

    scene->add(cylinder);
    // scene->integrate(scene2);
    scene->commit();

    

    // now cylinder is attached to two scenes
    std::cout << "ID -> SCENE" << std::endl;
    auto ids = cylinder->ids();
    for(auto elem : ids)
    {
        std::cout << "- " << elem.first;
        if(elem.second.lock() == scene)
        {
            std::cout << " -> scene 1";
        } else if(elem.second.lock() == scene2) {
            std::cout << " -> scene 2";
        }
        std::cout << std::endl;
    }


    printRaycast(scene, {0.0, 0.0, 50.0}, {1.0, 0.0, 0.0});
    printRaycast(scene2, {0.0, 0.0, 50.0}, {1.0, 0.0, 0.0});




    // remove cylinder from scene1

    scene->remove(cylinder);
    scene->commit();

    std::cout << "Cylinder removed from scene 1" << std::endl;

    std::cout << "ID -> SCENE" << std::endl;
    ids = cylinder->ids();
    for(auto elem : ids)
    {
        std::cout << "- " << elem.first;
        if(elem.second.lock() == scene)
        {
            std::cout << " -> scene 1";
        } else if(elem.second.lock() == scene2) {
            std::cout << " -> scene 2";
        }
        std::cout << std::endl;
    }
}

struct MyStruct;
using MyStructPtr = std::shared_ptr<MyStruct>;
using MyStructWPtr = std::weak_ptr<MyStruct>;




struct MyCompare {
    bool operator() (
        const MyStructWPtr &lhs, 
        const MyStructWPtr &rhs) const 
    {
        auto lptr = lhs.lock(), rptr = rhs.lock();
        if (!rptr) return false; // nothing after expired pointer 
        if (!lptr) return true;  // every not expired after expired pointer
        // why?
        return lptr != rptr;
        // return lptr == rptr;
    }
};

using MyWSet = std::set<MyStructWPtr, MyCompare>;



struct Data 
{
    MyWSet parents;
};

using DataPtr = std::shared_ptr<Data>;

struct MyStruct : public std::enable_shared_from_this<MyStruct>
{

    void removeChild()
    {
        if(child)
        {
            std::cout << "removed " 
            << child->parents.erase(weak_from_this())
            << std::endl;
        }
    }

    void setChild(DataPtr data)
    {
        child = data;
        data->parents.insert(weak_from_this());
    }

    DataPtr child;
};


void test()
{
    DataPtr bla = std::make_shared<Data>();

    MyStructPtr p1 = std::make_shared<MyStruct>();
    p1->setChild(bla);

    MyStructPtr p2 = std::make_shared<MyStruct>();
    p2->setChild(bla);

    std::cout << bla->parents.size() << std::endl;

    p1->removeChild();

    std::cout << bla->parents.size() << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << "Rmagine Embree Scene Building" << std::endl;
    

    

    // test();
    // return 0;




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
        case 5: scene_5(); break;
        case 6: scene_6(); break;
        case 7: scene_7(); break;
        case 8: scene_8(); break;
        case 9: scene_9(); break;
        case 10: scene_10(); break;
        case 11: scene_11(); break;
        default: break;
    }

    return 0;
}
