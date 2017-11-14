#ifndef KMUVCL_GRAPHICS_OPERATOR_HPP
#define KMUVCL_GRAPHICS_OPERATOR_HPP

#include "vec.hpp"
#include "mat.hpp"

namespace kmuvcl {
  namespace math {

    /// y_n = s * x_n
    template <unsigned int N, typename T>
    vec<N, T> operator* (const T s, const vec<N, T>& x)
    {
      // TODO: Fill up this function properly
      vec<N, T> y;
      for(int i = 0; i < N; ++i)
      {
	       y(i) = s * x(i);
      }
      return  y;
    }

    /// s = u_n * v_n (dot product)
    template <unsigned int N, typename T>
    T dot(const vec<N, T>& u, const vec<N, T>& v)
    {
      // TODO: Fill up this function properly
      T val = 0;
      for(int i = 0; i < N; ++i)
      {
	       val += u(i) * v(i);
      }
      return  val;
    }

    /// w_3 = u_3 x v_3 (cross product, only for vec3)
    template <typename T>
    vec<3,T> cross(const vec<3, T>& u, const vec<3, T>& v)
    {
      // TODO: Fill up this function properly
      vec<3, T>  w;
      w(0) = u(1) * v(2) - u(2) * v(1);
      w(1) = u(2) * v(0) - u(0) * v(2);
      w(2) = u(0) * v(1) - u(1) * v(0);
      return  w;
    }

    /// y_m = A_{mxn} * x_n
    template <unsigned int M, unsigned int N, typename T>
    vec<M, T> operator* (const mat<M, N, T>& A, const vec<N, T>& x)
    {
      // TODO: Fill up this function properly
      vec<M, T>   y, col;
      for(int i = 0; i < N; ++i)
      {
	      A.get_ith_column(i, col);
        y += x(i) * col;
      }
      return  y;
    };

    /// y_n = x_m * A_{mxn}
    template <unsigned int M, unsigned int N, typename T>
    vec<N, T> operator* (const vec<M, T>& x, const mat<M, N, T>& A)
    {
      // TODO: Fill up this function properly
      vec<N, T>   y;
      vec<M, T>   col;
      for(int i = 0; i < N; ++i)
      {
	      A.get_ith_column(i, col);
        y(i) = dot(col, x);
      }
      return  y;
    }

    /// C_{mxl} = A_{mxn} * B_{nxl}
    template <unsigned int M, unsigned int N, unsigned int L, typename T>
    mat<M, L, T> operator* (const mat<M, N, T>& A, const mat<N, L, T>& B)
    {
      // TODO: Fill up this function properly
      mat<M, L, T>   C;
      vec<N, T> col;
      vec<N, T> row;
      for(int i = 0; i < M; ++i)
      {
	       A.get_ith_row(i, row);
	       for(int j = 0; j < L; ++j)
	       {
           B.get_ith_column(j, col);
	         C(i, j) = dot(row, col);
	       }
      }
      return  C;
    }

    /// ostream for vec class
    template <unsigned int N, typename T>
    std::ostream& operator << (std::ostream& os, const vec<N, T>& v)
    {
      // TODO: Fill up this function properly
      os << "[";
      for(int i=0; i < N - 1; ++i)
      {
	       os << v(i) << ", ";
      }
      os << v(N-1);
      os << "]";

      return  os;
    }

    /// ostream for mat class
    template <unsigned int M, unsigned int N, typename T>
    std::ostream& operator << (std::ostream& os, const mat<M, N, T>& A)
    {
      // TODO: Fill up this function properly
      for(int i = 0; i < M; ++i)
      {
	       os << "[";
	       for(int j = 0; j < N - 1; ++j)
	       {
		         os << A(i, j) << ", ";
	       }
	       os << A(i, N - 1);
	       os << "]";
	       os << std::endl;
      }
      return  os;
    }
  } // math
} // kmuvcl

#endif // KMUVCL_GRAPHICS_OPERATOR_HPP
