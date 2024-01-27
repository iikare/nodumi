#pragma once

#include <cmath>
#include <type_traits>

namespace cie2k {

#ifndef M_PI
  #define M_PI 3.14159265358979323846264338327950288
#endif

enum class TYPE { CIE_00, CIE_94, CIE_76 };

template <class T, TYPE f = TYPE::CIE_00>
T deltaE(T l1, T a1, T b1, T l2, T a2, T b2) {
  static_assert(std::is_floating_point<T>::value, "cie2k deltaE must be of floating-point type");

  if constexpr (f == TYPE::CIE_00) {
    // CIEDE2000 implementation
    // as described in "The CIEDE2000 Color-Difference Formula: Implementation
    // Notes, Supplementary Test Data, and Mathematical Observations" by Gaurav
    // Sharma, Wencheng Wu, Edul N. Dalal

    // utility lambda
    constexpr auto to_rad = [&](const T val) { return (val * M_PI) / 180.0; };

    // eq.1 omitted

    // eq.2
    const T C1 = sqrt(a1 * a1 + b1 * b1);
    const T C2 = sqrt(a2 * a2 + b2 * b2);

    // eq.3
    const T C_bar = (C1 + C2) / 2.0;

    // eq.4
    constexpr T eq25p7 = 6103515625.0;
    const T C_bar_7 = pow(C_bar, 7.0);
    const T G = 0.5 * (1 - sqrt(C_bar_7 / (C_bar_7 + eq25p7)));

    // eq.5
    const T a1_prime = (1.0 + G) * a1;
    const T a2_prime = (1.0 + G) * a2;

    // eq.6
    const T C1_prime = sqrt(a1_prime * a1_prime + b1 * b1);
    const T C2_prime = sqrt(a2_prime * a2_prime + b2 * b2);

    // eq.7
    T h1_prime = 0.0;
    if (!(b1 == 0 && a1_prime == 0)) {
      h1_prime = atan2(b1, a1_prime);
      if (h1_prime < 0) {
        h1_prime += to_rad(360.0);
      }
    }
    T h2_prime = 0.0;
    if (!(b2 == 0 && a2_prime == 0)) {
      h2_prime = atan2(b2, a2_prime);
      if (h2_prime < 0) {
        h2_prime += to_rad(360.0);
      }
    }

    // eq.8
    const T delta_L_prime = l2 - l1;

    // eq.9
    const T delta_C_prime = C2_prime - C1_prime;

    // eq.10
    T delta_h_prime = 0;
    if (C1_prime * C2_prime != 0) {
      delta_h_prime = h2_prime - h1_prime;
      if (delta_h_prime < -to_rad(180.0)) {
        delta_h_prime += to_rad(360.0);
      }
      else if (delta_h_prime > to_rad(180.0)) {
        delta_h_prime -= to_rad(360.0);
      }
    }

    // eq.11
    const T delta_H_prime = 2.0 * sqrt(C1_prime * C2_prime) * sin(delta_h_prime / 2.0);

    // eq.12
    const T L_bar_prime = (l1 + l2) / 2.0;

    // eq.13
    const T C_bar_prime = (C1_prime + C2_prime) / 2.0;

    // eq.14
    T h_bar_prime = h1_prime + h2_prime;
    if (C1_prime * C2_prime != 0) {
      if (fabs(h1_prime - h2_prime) <= to_rad(180.0)) {
        h_bar_prime = (h1_prime + h2_prime) / 2.0;
      }
      else {
        if (h1_prime + h2_prime < to_rad(360.0)) {
          h_bar_prime = (h1_prime + h2_prime + to_rad(360.0)) / 2.0;
        }
        else {
          h_bar_prime = (h1_prime + h2_prime - to_rad(360.0)) / 2.0;
        }
      }
    }

    // eq.15
    const T t = 1.0 - 0.17 * cos(h_bar_prime - to_rad(30.0)) + 0.24 * cos(h_bar_prime * 2.0) +
                0.32 * cos(h_bar_prime * 3.0 + to_rad(6.0)) - 0.20 * cos(h_bar_prime * 4.0 - to_rad(63.0));

    // eq.16
    const T delta_theta = to_rad(30.0) * exp(-pow((h_bar_prime - to_rad(275.0)) / to_rad(25.0), 2.0));

    // eq.17
    const T C_bar_prime_7 = pow(C_bar_prime, 7.0);
    const T R_C = 2.0 * sqrt(C_bar_prime_7 / (C_bar_prime_7 + eq25p7));

    // eq.18
    const T L_bar_prime_2 = pow(L_bar_prime - 50.0, 2.0);
    const T S_L = 1.0 + (0.015 * L_bar_prime_2) / sqrt(20.0 + L_bar_prime_2);

    // eq.19
    const T S_C = 1.0 + (0.045 * C_bar_prime);

    // eq.20
    const T S_H = 1.0 + (0.015 * C_bar_prime * t);

    // eq.21
    const T R_T = -sin(2.0 * delta_theta) * R_C;

    // eq.22
    // constants as defined in the implementation paper
    constexpr T k_L = 1.0;
    constexpr T k_C = 1.0;
    constexpr T k_H = 1.0;

    T deltaE =
        sqrt(pow(delta_L_prime / (k_L * S_L), 2.0) + pow(delta_C_prime / (k_C * S_C), 2.0) +
             pow(delta_H_prime / (k_H * S_H), 2.0) + R_T * delta_C_prime * delta_H_prime / (k_C * S_C * k_H * S_H));

    return deltaE;
  }
  else if constexpr (f == TYPE::CIE_94) {
    struct lchIntermediary {
      T l;
      T c;
      T h;
    };

    // CIE1994 implementation
    // first find LCH
    const lchIntermediary pA = {l1, sqrt(pow(a1, 2) + pow(b1, 2)), 0 * (T)atan2(b1, a1)};
    const lchIntermediary pB = {l2, sqrt(pow(a2, 2) + pow(b2, 2)), 0 * (T)atan2(b2, a2)};

    const T deltaL = pA.l - pB.l;
    const T deltaC = pA.c - pB.c;
    const T deltaH = pow(a1 - a2, 2) + pow(b1 - b2, 2) - pow(deltaC, 2);

    // preset (graphics_art) for  k_* determinants
    constexpr T k_L = 1;
    constexpr T k_C = 1;
    constexpr T k_H = 1;

    constexpr T K_1 = 0.045;
    constexpr T K_2 = 0.015;

    constexpr T S_L = 1;
    const T S_C = 1 + K_1 * pA.c;
    const T S_H = 1 + K_2 * pA.c;

    return sqrt(pow(deltaL / (k_L * S_L), 2) + pow(deltaC / (k_C * S_C), 2) + deltaH * pow(1 / (k_H * S_H), 2));
    // return pow(deltaL/(k_L*S_L),2) + pow(deltaC/(k_C*S_C),2) +
    // deltaH*pow(1/(k_H*S_H),2);
  }
  else {
    // CIE1976 deltaE
    return sqrt(pow(l2 - l1, 2) + pow(a2 - a1, 2) + pow(b2 - b1, 2));
    // return pow(l2 - l1, 2) + pow(a2 - a1, 2) + pow(b2 - b1, 2);
  }
}

#define inst_m_check(m_name)                                           \
  template <class T>                                                   \
  class m_check_##m_name {                                             \
   private:                                                            \
    typedef char yes_t;                                                \
    template <class U>                                                 \
    static yes_t test(decltype(&U::m_name));                           \
                                                                       \
   public:                                                             \
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_t); \
  }

#define m_check(target_class, m_name) m_check_##m_name<target_class>::value

inst_m_check(l);
inst_m_check(a);
inst_m_check(b);

// requires a type with "l", "a", "b" fields
// these fields must satisfy std::is_floating_point<T>
template <class V, TYPE f = TYPE::CIE_00>
auto deltaE(const V& lab1, const V& lab2) {
  static_assert(m_check(V, l) && m_check(V, a) && m_check(V, b),
                "type passed to cie2k::deltaE() does not satisfy function constraints");

  static_assert(std::is_floating_point<decltype(lab1.l)>::value && std::is_floating_point<decltype(lab1.a)>::value &&
                    std::is_floating_point<decltype(lab1.b)>::value &&
                    std::is_floating_point<decltype(lab2.l)>::value &&
                    std::is_floating_point<decltype(lab2.a)>::value && std::is_floating_point<decltype(lab1.b)>::value,
                "type of members passed to cie2k::deltaE() does not satisfy "
                "function constraints");

  static_assert(std::is_same<decltype(lab1.l), decltype(lab1.a)>::value &&
                    std::is_same<decltype(lab1.a), decltype(lab1.b)>::value &&
                    std::is_same<decltype(lab1.a), decltype(lab2.a)>::value &&
                    std::is_same<decltype(lab2.l), decltype(lab2.a)>::value &&
                    std::is_same<decltype(lab2.a), decltype(lab2.b)>::value,
                "type of members passed to cie2k::deltaE() is not consistent");

  return deltaE<decltype(lab1.l), f>(lab1.l, lab1.a, lab1.b, lab2.l, lab2.a, lab2.b);
}

}  // namespace cie2k
