#pragma once 

// compatability type since raylib doesn't provide GL blend attributes
enum gl_compat {
  /* Accumulation buffer */
  CGL_ACCUM_RED_BITS      = 0x0D58,
  CGL_ACCUM_GREEN_BITS      = 0x0D59,
  CGL_ACCUM_BLUE_BITS       = 0x0D5A,
  CGL_ACCUM_ALPHA_BITS      = 0x0D5B,
  CGL_ACCUM_CLEAR_VALUE       = 0x0B80,
  CGL_ACCUM         = 0x0100,
  CGL_ADD           = 0x0104,
  CGL_LOAD          = 0x0101,
  CGL_MULT          = 0x0103,
  CGL_RETURN        = 0x0102,

  /* Alpha testing */
  CGL_ALPHA_TEST        = 0x0BC0,
  CGL_ALPHA_TEST_REF      = 0x0BC2,
  CGL_ALPHA_TEST_FUNC       = 0x0BC1,

  /* Blending */
  CGL_BLEND         = 0x0BE2,
  CGL_BLEND_SRC         = 0x0BE1,
  CGL_BLEND_DST         = 0x0BE0,
  CGL_ZERO          = 0,
  CGL_ONE          = 1,
  CGL_SRC_COLOR         = 0x0300,
  CGL_ONE_MINUS_SRC_COLOR       = 0x0301,
  CGL_SRC_ALPHA         = 0x0302,
  CGL_ONE_MINUS_SRC_ALPHA       = 0x0303,
  CGL_DST_ALPHA         = 0x0304,
  CGL_ONE_MINUS_DST_ALPHA       = 0x0305,
  CGL_DST_COLOR         = 0x0306,
  CGL_ONE_MINUS_DST_COLOR       = 0x0307,
  CGL_SRC_ALPHA_SATURATE      = 0x0308,
};