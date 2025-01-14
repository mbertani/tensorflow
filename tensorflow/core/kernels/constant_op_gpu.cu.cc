/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#if (defined(GOOGLE_CUDA) && GOOGLE_CUDA) || \
    (defined(TENSORFLOW_USE_ROCM) && TENSORFLOW_USE_ROCM)

#define EIGEN_USE_GPU

#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/framework/tensor_types.h"
#include "tensorflow/core/kernels/fill_functor.h"
#include "tensorflow/core/platform/types.h"

namespace Eigen {
namespace internal {

template <typename T>
struct scalar_const_op {
  typedef typename packet_traits<T>::type Packet;

  const T* val;

  EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE
  scalar_const_op(const scalar_const_op& x)
      : val(x.val) {}

  EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE scalar_const_op(const T* v) : val(v) {}

  EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const T operator()() const {
    return *val;
  }

  template <typename PacketType = Packet>
  EIGEN_DEVICE_FUNC EIGEN_STRONG_INLINE const PacketType packetOp() const {
    return internal::pset1<PacketType>(*val);
  }
};

template <typename T>
struct functor_traits<scalar_const_op<T> > {
  enum {
    Cost = 1,
    PacketAccess = packet_traits<T>::Vectorizable,
    IsRepeatable = true
  };
};

}  // end namespace internal
}  // end namespace Eigen

namespace tensorflow {

namespace functor {

typedef Eigen::GpuDevice GPUDevice;

// Partial specialization FillFunctor<Device=GPUDevice, T>
template <typename T>
struct FillFunctor<GPUDevice, T> {
  void operator()(const GPUDevice& d, typename TTypes<T>::Flat out,
                  typename TTypes<T>::ConstScalar in) {
    Eigen::internal::scalar_const_op<T> f(in.data());
    MaybeWith32BitIndexing<GPUDevice>(
        [&](auto out32) { out32.device(d) = out32.nullaryExpr(f); }, out);
  }
};

#define DEFINE_FILL_GPU(T) template struct FillFunctor<GPUDevice, T>;
TF_CALL_REAL_NUMBER_TYPES(DEFINE_FILL_GPU);
TF_CALL_bool(DEFINE_FILL_GPU);
#undef DEFINE_FILL_GPU

// Partial specialization of FillFunctor<Device=GPUDevice, T>.
template <typename T>
struct SetZeroFunctor<GPUDevice, T> {
  void operator()(const GPUDevice& d, typename TTypes<T>::Flat out) {
    MaybeWith32BitIndexing<GPUDevice>(
        [&](auto out32) { out32.device(d) = out32.constant(T(0)); }, out);
  }
};

#define DEFINE_SETZERO_GPU(T) template struct SetZeroFunctor<GPUDevice, T>;

// The SetZeroFunctor is called from several other ops, so even though on GPU we
// are using a MLIR generated ZerosLike kernel, we still need to define this
// functor for the other ops.
TF_CALL_bool(DEFINE_SETZERO_GPU);
TF_CALL_int64(DEFINE_SETZERO_GPU);
TF_CALL_FLOAT_TYPES(DEFINE_SETZERO_GPU);
TF_CALL_int32(DEFINE_SETZERO_GPU);
TF_CALL_COMPLEX_TYPES(DEFINE_SETZERO_GPU);
#undef DEFINE_SETZERO_GPU

// Partial specialization of FillFunctor<Device=GPUDevice, T>.
template <typename T>
struct SetOneFunctor<GPUDevice, T> {
  void operator()(const GPUDevice& d, typename TTypes<T>::Flat out) {
    MaybeWith32BitIndexing<GPUDevice>(
        [&](auto out32) { out32.device(d) = out32.constant(T(1)); }, out);
  }
};

#define DEFINE_SETONE_GPU(T) template struct SetOneFunctor<GPUDevice, T>;
#if !defined(MLIR_GENERATED_GPU_KERNELS_ENABLED) || \
    !defined(MLIR_GENERATED_EXPERIMENTAL_KERNELS_ENABLED)
TF_CALL_bool(DEFINE_SETONE_GPU);
TF_CALL_int64(DEFINE_SETONE_GPU);
TF_CALL_FLOAT_TYPES(DEFINE_SETONE_GPU);
#else
TF_CALL_bfloat16(DEFINE_SETONE_GPU);
#endif
TF_CALL_int32(DEFINE_SETONE_GPU);
TF_CALL_COMPLEX_TYPES(DEFINE_SETONE_GPU);
#undef DEFINE_SETONE_GPU

}  // end namespace functor
}  // end namespace tensorflow

#endif  // GOOGLE_CUDA || TENSORFLOW_USE_ROCM
