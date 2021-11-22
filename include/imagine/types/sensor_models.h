#ifndef IMAGINE_TYPES_SENSOR_MODELS_H
#define IMAGINE_TYPES_SENSOR_MODELS_H

#include <imagine/math/types.h>
#include <cstdint>
#include <math.h>

#ifdef __CUDA_ARCH__
#define IMAGINE_FUNCTION __host__ __device__
#define IMAGINE_INLINE_FUNCTION __inline__ __host__ __device__ 
#else
#define IMAGINE_FUNCTION
#define IMAGINE_INLINE_FUNCTION inline
#endif


namespace imagine
{

struct Interval {
    float min;
    float max;
};

struct DiscreteInterval
{
    float min;
    float max;
    float step;
    uint32_t size;

    /**
    * @brief compute step value from given others
    */
    IMAGINE_INLINE_FUNCTION
    void computeStep()
    {
        step = (max - min) / ( static_cast<float>(size - 1) );
    }

    IMAGINE_INLINE_FUNCTION
    float getValue(uint32_t id) const
    {
        return min + static_cast<float>(id) * step;
    }
};

struct Rectangle {
    Vector2 min;
    Vector2 max;
};

struct Box {
    Vector min;
    Vector max;
};

struct SphericalModel 
{
    // PHI: vertical, y-rot, pitch, polar angle 
    DiscreteInterval phi;
    // THETA: horizontal, z-rot, yaw, azimuth
    DiscreteInterval theta;
    // RANGE: range
    Interval range; // range is valid if <= range_max && >= range_min

    IMAGINE_INLINE_FUNCTION
    float getPhi(uint32_t phi_id) const
    {
        return phi.getValue(phi_id);
    }

    IMAGINE_INLINE_FUNCTION
    float getTheta(uint32_t theta_id) const
    {
        return theta.getValue(theta_id);
    }

    IMAGINE_INLINE_FUNCTION
    Vector getRay(uint32_t phi_id, uint32_t theta_id) const
    {
        const float phi_ = getPhi(phi_id);
        const float theta_ = getTheta(theta_id);
        return {cosf(phi_) * cosf(theta_), cosf(phi_) * sinf(theta_), sinf(phi_)};
    }

    IMAGINE_INLINE_FUNCTION
    uint32_t getBufferId(uint32_t phi_id, uint32_t theta_id) const 
    {
        return phi_id * theta.size + theta_id;
    }
};

using LiDARModel = SphericalModel;

struct CylindricModel {
    // TODO
};

struct PinholeModel {
    uint32_t width;
    uint32_t height;

    // maximum and minimum allowed range
    Interval range;

    // Focal length fx and fy
    float f[2];
    // Center cx and cy
    float c[2];
};
// Distortion? Fisheye / radial-tangential ? 

using CameraModel = PinholeModel;
using DepthCameraModel = PinholeModel;

struct RadialTangentialDistortion {
    // TODO
};

struct FisheyeDistortion {

};

} // namespace imagine

#endif // IMAGINE_TYPES_SENSOR_MODELS_H