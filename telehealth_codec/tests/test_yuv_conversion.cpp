#include <codec/Frame.h>
#include <codec/YuvConverter.h>
#include <cmath>
#include <cstring>
#include <iostream>

double compute_psnr(const uint8_t* a, const uint8_t* b, size_t size) {
  double mse = 0;
  for (size_t i = 0; i < size; ++i) {
    double d = static_cast<double>(a[i]) - static_cast<double>(b[i]);
    mse += d * d;
  }
  mse /= size;
  if (mse <= 0) return 99.0;
  return 10.0 * std::log10(255.0 * 255.0 / mse);
}

int main() {
  telehealth::codec::FrameRGB rgb(64, 64);
  for (int y = 0; y < 64; ++y)
    for (int x = 0; x < 64; ++x) {
      rgb.row(y)[x * 3]     = static_cast<uint8_t>((x + y) % 256);
      rgb.row(y)[x * 3 + 1] = static_cast<uint8_t>((x * 2 + y) % 256);
      rgb.row(y)[x * 3 + 2] = static_cast<uint8_t>(y % 256);
    }

  telehealth::codec::FrameYUV yuv;
  telehealth::codec::YuvConverter conv;
  conv.rgb_to_yuv420(rgb, yuv);

  if (yuv.width != 64 || yuv.height != 64) {
    std::cerr << "YUV dimensions wrong\n";
    return 1;
  }

  telehealth::codec::FrameRGB rgb2;
  conv.yuv420_to_rgb(yuv, rgb2);
  double psnr = compute_psnr(rgb.data.data(), rgb2.data.data(), rgb.data.size());
  if (psnr < 25.0) {
    std::cerr << "PSNR too low after roundtrip: " << psnr << "\n";
    return 1;
  }
  std::cout << "YUV conversion test OK (PSNR=" << psnr << " dB)\n";
  return 0;
}
