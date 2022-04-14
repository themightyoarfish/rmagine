#ifndef RMAGINE_MATH_SVD_HPP
#define RMAGINE_MATH_SVD_HPP

#include <rmagine/types/Memory.hpp>
#include <rmagine/math/types.h>
#include <memory>

namespace rmagine {

class SVD
{
public:
    SVD();

    ~SVD();

    void calcUV(
        const Matrix3x3& A,
        Matrix3x3& U,
        Matrix3x3& V
    ) const;

    void calcUSV(const Matrix3x3& A,
        Matrix3x3& U,
        Vector& S,
        Matrix3x3& V
    ) const;

    void calcUV(
        const Memory<Matrix3x3, RAM>& As,
        Memory<Matrix3x3, RAM>& Us,
        Memory<Matrix3x3, RAM>& Vs
    ) const;

    void calcUSV(const Memory<Matrix3x3, RAM>& As,
        Memory<Matrix3x3, RAM>& Us,
        Memory<Vector, RAM>& Ss,
        Memory<Matrix3x3, RAM>& Vs) const;
};

using SVDPtr = std::shared_ptr<SVD>;

} // namespace rmagine

#endif // RMAGINE_MATH_SVD_HPP