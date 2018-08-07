/*
*  Anand Hotwani 2018
*
*  Simple Command Line program to export out .png images for review and reference from scene-linear HDR .exr files.
*  Written to avoid relying packages such as Nuke or Photoshop so that this can be integrated into a wider ingestion pipeline
*  
*  Instructions to build:
*     
*	$ mkdir build
*	$ cd build
*	$ cmake ..
*	$ make
*
*	Instructions to run
*
*	$ ./ahBakeTransform <input.exr> <output.png> <colorspace> <resize>
*	
*	e.g.  $ ./ahBakeTransform test.exr test_sRGB_ACES.png 3 1
*
*	Will produce a .png file that takes the scene-linear .exr file and outputs a baked ACES RRT+ODT suitable for LDR viewing on an sRGB device.
*
* 
*      
*/
#include <cstdio>
#include <cstdlib>
#include <vector>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include "geometry.h"

inline float clamp(const float &lo, const float &hi, const float &v) { return std::max(lo, std::min(hi, v)); } 


Vector3f RRTAndODTFit(Vector3f v) {
	
	Vector3f a = v * (v + 0.0245786f) - 0.000090537f;
    Vector3f b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return Vector3f((a.x / b.x), (a.y / b.y), (a.z / b.z));
	
}

// Curve fitted approximation by Stephen Hill (@self_shadow). Good appoximation though slightly oversaturates compared to CTL transform.
Vector3f ACESFitted(Vector3f color) {
		
	color.x = color.x * 0.59719 + color.y * 0.35458 + color.z * 0.04823;
	color.y = color.x * 0.07600 + color.y * 0.90834 + color.z * 0.01566;
	color.z = color.x * 0.02840 + color.y * 0.13383 + color.z * 0.83777;

	color = RRTAndODTFit(color);
	
	color.x = color.x * 1.60475 + color.y * -0.53108 + color.z * -0.07367;
	color.y = color.x * -0.10208 + color.y *  1.10813 + color.z * -0.00605;
	color.z = color.x * -0.00327 + color.y * -0.07276 + color.z *  1.07602;
	
	return color;
}

inline unsigned char FloatToUnsignedChar(float f)
{
  int i = static_cast<int>(255.0f * f);
  if (i > 255) i = 255;
  if (i < 0) i = 0;

  return static_cast<unsigned char>(i);
}

inline float ApplysRGBCurve(float x)
{
	return x < 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

inline float ApplyRec709Curve(float x)
{
	return x < 0.0181 ? 4.5 * x : 1.0993 * pow(x, 0.45) - 0.0993;
}

bool SaveImage(const char* filename, const float* rgba, int colorspace, int width, int height) {

  std::vector<unsigned char> dst(width * height * 4);

	// Color transform baked in depending on int flags
	
	// Linear
	if (colorspace == (int)0) {
		
		for (size_t i = 0; i < width * height; i++) {
			dst[i * 4 + 0] = FloatToUnsignedChar((rgba[i * 4 + 0]));
			dst[i * 4 + 1] = FloatToUnsignedChar((rgba[i * 4 + 1]));
			dst[i * 4 + 2] = FloatToUnsignedChar((rgba[i * 4 + 2]));
			dst[i * 4 + 3] = 255;	// Alpha
		}
	}
	
	// sRGB
	if (colorspace == (int)1) {
		
			for (size_t i = 0; i < width * height; i++) {
				dst[i * 4 + 0] = FloatToUnsignedChar(ApplysRGBCurve(rgba[i * 4 + 0]));
				dst[i * 4 + 1] = FloatToUnsignedChar(ApplysRGBCurve(rgba[i * 4 + 1]));
				dst[i * 4 + 2] = FloatToUnsignedChar(ApplysRGBCurve(rgba[i * 4 + 2]));
				dst[i * 4 + 3] = 255;	// Alpha
		}
	}
	
	// Rec. 709
	if (colorspace == (int)2) {
		
			for (size_t i = 0; i < width * height; i++) {
				dst[i * 4 + 0] = FloatToUnsignedChar(ApplyRec709Curve(rgba[i * 4 + 0]));
				dst[i * 4 + 1] = FloatToUnsignedChar(ApplyRec709Curve(rgba[i * 4 + 1]));
				dst[i * 4 + 2] = FloatToUnsignedChar(ApplyRec709Curve(rgba[i * 4 + 2]));
				dst[i * 4 + 3] = 255;	// Alpha
		}
	}
		
	if (colorspace == (int)3) {
		
			for (size_t i = 0; i < width * height; i++) {
				
				dst[i * 4 + 0] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[0]));
				dst[i * 4 + 1] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[1]));
				dst[i * 4 + 2] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[2]));
				dst[i * 4 + 3] = 255;	// Alpha
		}
	}
	
	if (colorspace == (int)4) {
		
			for (size_t i = 0; i < width * height; i++) {
				
				dst[i * 4 + 0] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[0]));
				dst[i * 4 + 1] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[1]));
				dst[i * 4 + 2] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[2]));
				dst[i * 4 + 3] = 255;	// Alpha
		}
	}
	
	if (colorspace == (int)5) {
		
			for (size_t i = 0; i < width * height; i++) {
				
				dst[i * 4 + 0] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[0]));
				dst[i * 4 + 1] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[1]));
				dst[i * 4 + 2] = FloatToUnsignedChar(ApplysRGBCurve(ACESFitted(Vector3f(rgba[i * 4 + 0], rgba[i * 4 + 1], rgba[i * 4 + 2]))[2]));
				dst[i * 4 + 3] = 255;	// Alpha
		}
	}
	
  int image = stbi_write_png(filename, width, height, 4, static_cast<const void*>(dst.data()), width * 4);

  return (image > 0);
}

int main(int argc, char** argv)
{
  if (argc < 3) {
	printf("\n\n>>> ahBurnGrade - A Tool For Processing HDR .exr files to output LDR .pngs for previewing (reference images)\n\n");
    printf("    -Example: ./ahBurnGrade input.exr output.png [colorspace] [resize] \n\n");
    printf("        Colorspace:  Default is 1. Linear        = 0\n");
	printf("                                   sRGB          = 1\n");
	printf("                                   Rec. 709      = 2\n");
	printf("                                   ACES sRGB     = 3\n");
	printf("                                   ACES Rec. 709 = 4\n");
    printf("                                   ACES DCI-P3   = 5\n\n");
    printf("        Resize    :  Default is 1. Scaling factor to reduce image size. 2 will produce half-sized image.\n");
    exit(-1);
  }

  int colorspace = 1;
  if (argc > 3) {
    colorspace = atoi(argv[3]);
  }

  float resize_factor = 1.0f;
  if (argc > 4) {
    resize_factor = atof(argv[4]);
  }

  int width, height;
  float* rgba;
  const char* err;

  {
    int image = LoadEXR(&rgba, &width, &height, argv[1], &err);
    if (image != 0) {
      printf("ERROR: %s\n", err);
      return -1;
    }
  }

  int dst_width  = width / resize_factor;
  int dst_height = height / resize_factor;
  printf(">>> Output resolution will be = %d x %d\n", dst_width, dst_height);

  std::vector<float> buf(dst_width * dst_height * 4);
  int image = stbir_resize_float(rgba, width, height, width*4*sizeof(float), &buf.at(0), dst_width, dst_height,dst_width*4*sizeof(float), 4);
  assert(image != 0);

  printf(">>> Writing out image...\n");
  bool success = SaveImage(argv[2], &buf.at(0), colorspace, dst_width, dst_height);

  printf(">>> Completed!\n");
  return (success ? 0 : -1);
}
