#pragma once
#include <vector>

class ColorTransformer {
public:
    ColorTransformer(int brightness, int contrast)
        : rgb_table_(65536)
    {
        int multiply;
        if (contrast < 0) {
            multiply = contrast + 100;
            divide_ = 100;
        }
        else if (contrast > 0) {
            multiply = 100;
            divide_ = 100 - contrast;
        }
        else {
            multiply = 1;
            divide_ = 1;
        }


        if (divide_ == 0) {
            for (int intensity = 0; intensity < 256; ++intensity) {
                if (intensity + brightness < 128) {
                    rgb_table_[intensity] = 0;
                }
                else {
                    rgb_table_[intensity] = 255;
                }
            }
        }
        else if (divide_ == 100) {
            for (int intensity = 0; intensity < 256; ++intensity) {
                int shift = (intensity - 127) * multiply / divide_ + 127 - intensity + brightness;
                for (int col = 0; col < 256; ++col) {
                    int index = (intensity * 256) + col;
                    rgb_table_[index] = ClampToByte(col + shift);
                }
            }
        }
        else {
            for (int intensity = 0; intensity < 256; ++intensity) {
                int shift = (intensity - 127 + brightness) * multiply / divide_ + 127 - intensity;
                for (int col = 0; col < 256; ++col) {
                    int index = (intensity * 256) + col;
                    rgb_table_[index] = ClampToByte(col + shift);
                }
            }
        }
    }

    inline unsigned char ClampToByte(int value) {
        if (value < 0) {
            return 0;
        }
        else if (value > 255) {
            return 255;
        }
        else {
            return value;
        }
    }

    inline unsigned char ConvertChannel(unsigned char c, int intensity) {
        if (divide_ == 0) {
            return rgb_table_[intensity];
        }
        else {
            return rgb_table_[intensity * 256 + c];
        }
    }

    inline static unsigned char GetIntensityByte(unsigned char r, unsigned char g, unsigned char b)
    {
        // analogue to return ((0.114 * (double)B) + (0.587 * (double)G) + (0.299 * (double)R)) / 255.0;
        // From Paint.NET sources
        // https://github.com/rivy/OpenPDN/blob/cca476b0df2a2f70996e6b9486ec45327631568c/src/Core/ColorBgra.cs#L132
        return (unsigned char)((7471 * b + 38470 * g + 19595 * r) >> 16);
    }

#ifdef ANNO_USE_OPENCV
    inline cv::Vec3b Transform(const cv::Vec3b &color) {
        auto intensity = GetIntensityByte(color[0], color[1], color[2]);
        return cv::Vec3b(
            ConvertChannel(color[0], intensity),
            ConvertChannel(color[1], intensity),
            ConvertChannel(color[2], intensity));
    }
#endif

    inline void TransformB3(unsigned char *p) {
        auto intensity = GetIntensityByte(p[0], p[1], p[2]);
        p[0] = ConvertChannel(p[0], intensity);
        p[1] = ConvertChannel(p[1], intensity);
        p[2] = ConvertChannel(p[2], intensity);
    }

private:
    std::vector<unsigned char> rgb_table_;
    int divide_;
};
