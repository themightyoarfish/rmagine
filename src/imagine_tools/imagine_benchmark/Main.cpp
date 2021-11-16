#include <iostream>

// General mamcl includes
#include <imagine/types/sensor_models.h>
#include <imagine/util/StopWatch.hpp>

// CPU
#include <imagine/simulation/EmbreeSimulator.hpp>
#include <imagine/types/Memory.hpp>

// GPU
#if defined WITH_OPTIX
#include <imagine/simulation/OptixSimulator.hpp>
#include <imagine/types/MemoryCuda.hpp>
#endif

#include <iomanip>

using namespace imagine;

Memory<LiDARModel, RAM> velodyne_model()
{
    Memory<LiDARModel, RAM> model;
    model->theta.min = -M_PI;
    model->theta.max = M_PI; 
    model->theta.size = 440;
    model->theta.step = (model->theta.max - model->theta.min) / ( static_cast<float>(model->theta.size - 1) );
    
    model->phi.min = -M_PI;
    model->phi.max = M_PI;
    model->phi.size = 16;
    // automate this somehow?
    model->phi.step = (model->phi.max - model->phi.min) / ( static_cast<float>(model->phi.size - 1) );
    
    model->range.min = 1.0;
    model->range.max = 100.0;
    return model;
}

int main(int argc, char** argv)
{
    std::cout << "Imagine Benchmark" << std::endl;

    // Total runtime of the Benchmark in seconds
    double benchmark_duration = 10.0;
    // Poses to check per call
    size_t Nposes = 1024 * 10;

    // minimum 2 arguments
    if(argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " mesh_file device [device_id]" << std::endl;

        return 0;
    }

    std::string path_to_mesh = argv[1];
    std::string device = argv[2];
    int device_id = 0;
    
    if(argc > 3)
    {
        // device id specified
        device_id = atoi(argv[3]);
    }

    std::cout << "Inputs: " << std::endl;
    std::cout << "- mesh: " << path_to_mesh << std::endl;
    std::cout << "- device: " << device << std::endl;
    std::cout << "- device_id: " << device_id << std::endl; 

    StopWatch sw;
    double elapsed;
    double elapsed_total;

    if(device == "cpu")
    {
        // Define one Transform Sensor to Base
        Memory<Transform, RAM> Tsb;
        Tsb->R.x = 0.0;
        Tsb->R.y = 0.0;
        Tsb->R.z = 0.0;
        Tsb->R.w = 1.0;
        Tsb->t.x = 0.0;
        Tsb->t.y = 0.0;
        Tsb->t.z = 0.0;


        // Define Transforms Base to Map (Poses)
        Memory<Transform, RAM> Tbm(Nposes);
        for(size_t i=0; i<Tbm.size(); i++)
        {
            Tbm[i] = Tsb[0];
        }

        // Get Sensor Model
        Memory<LiDARModel, RAM> model = velodyne_model();

        // Load mesh
        EmbreeMapPtr cpu_mesh = importEmbreeMap(path_to_mesh);
        // std::cout << "Mesh loaded to CPU." << std::endl;
        EmbreeSimulatorPtr cpu_sim(new EmbreeSimulator(cpu_mesh));
        // std::cout << "Initialized CPU simulator." << std::endl;

        cpu_sim->setTsb(Tsb);
        cpu_sim->setModel(model);

        // Define what to simulate

        double velos_per_second_mean = 0.0;

        std::cout << "- range of last ray: " << cpu_sim->simulateRanges(Tbm)[Tbm.size() * model->phi.size * model->theta.size - 1] << std::endl;
        std::cout << "-- Starting Benchmark --" << std::endl;

        int run = 0;
        while(elapsed_total < benchmark_duration)
        {
            double n_dbl = static_cast<double>(run) + 1.0;
            // Simulate
            sw();
            auto res = cpu_sim->simulateRanges(Tbm);
            elapsed = sw();
            elapsed_total += elapsed;
            double velos_per_second = static_cast<double>(Nposes) / elapsed;
            velos_per_second_mean = (n_dbl - 1.0)/(n_dbl) * velos_per_second_mean + (1.0 / n_dbl) * velos_per_second; 
            
            std::cout
            << "[ " << int((elapsed_total / benchmark_duration)*100.0) << "%" << " - " 
            << "velos/s: " << velos_per_second 
            << ", mean: " << velos_per_second_mean  << "] \r";
            std::cout.flush();

            run++;
        }
        std::cout << std::endl;
        std::cout << "Result: " << velos_per_second_mean << " velos/s" << std::endl;

    } else if(device == "gpu") {
        #if defined WITH_OPTIX

        // Define one Transform Sensor to Base
        Memory<Transform, RAM> Tsb;
        Tsb->R.x = 0.0;
        Tsb->R.y = 0.0;
        Tsb->R.z = 0.0;
        Tsb->R.w = 1.0;
        Tsb->t.x = 0.0;
        Tsb->t.y = 0.0;
        Tsb->t.z = 0.0;

        // Define Transforms Base to Map (Poses)
        Memory<Transform, RAM_CUDA> Tbm(Nposes);
        for(size_t i=0; i<Tbm.size(); i++)
        {
            Tbm[i] = Tsb[0];
        }

        // Get Sensor Model
        Memory<LiDARModel, RAM> model = velodyne_model();
        
        // Load mesh
        OptixMapPtr gpu_mesh = importOptixMap(path_to_mesh, device_id);
        OptixSimulatorPtr gpu_sim(new OptixSimulator(gpu_mesh));

        gpu_sim->setTsb(Tsb);
        gpu_sim->setModel(model);

        // upload
        Memory<Transform, VRAM_CUDA> Tbm_gpu;
        Tbm_gpu = Tbm;

        // Define what to simulate
        Memory<float, RAM_CUDA> ranges;
        ranges = gpu_sim->simulateRanges(Tbm_gpu);
        
        std::cout << "- range of last ray: " << ranges[Tbm.size() * model->phi.size * model->theta.size - 1] << std::endl;
        std::cout << "-- Starting Benchmark --" << std::endl;

        double velos_per_second_mean = 0.0;
        
        int run = 0;
        while(elapsed_total < benchmark_duration)
        {
            double n_dbl = static_cast<double>(run) + 1.0;
            // Simulate
            sw();
            auto res = gpu_sim->simulateRanges(Tbm_gpu);
            elapsed = sw();
            elapsed_total += elapsed;
            double velos_per_second = static_cast<double>(Nposes) / elapsed;
            velos_per_second_mean = (n_dbl - 1.0)/(n_dbl) * velos_per_second_mean + (1.0 / n_dbl) * velos_per_second; 
            
            std::cout
            << "[ " << int((elapsed_total / benchmark_duration)*100.0) << "%" << " - " 
            << "velos/s: " << velos_per_second 
            << ", mean: " << velos_per_second_mean  << "] \r";
            std::cout.flush();

            run++;
        }

        std::cout << std::endl;
        std::cout << "Result: " << velos_per_second_mean << " velos/s" << std::endl;
        #endif
    } else {
        std::cout << "Device " << device << " unknown" << std::endl;
    }

    return 0;
}
