// This file is part of the AliceVision project.
// Copyright (c) 2019 AliceVision contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "hdrMerge.hpp"
#include <cassert>
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <aliceVision/alicevision_omp.hpp>


namespace aliceVision {
namespace hdr {
  
void hdrMerge::process(const std::vector< image::Image<image::RGBfColor> > &images,
                              const std::vector<float> &times,
                              const rgbCurve &weight,
                              const rgbCurve &response,
                              image::Image<image::RGBfColor> &radiance,
                              float targetTime,
                              bool robCalibrate,
                              float clampedValueCorrection)
{
  //checks
  assert(!response.isEmpty());
  assert(!images.empty());
  assert(images.size() == times.size());

  //reset radiance image
  radiance.fill(image::RGBfColor(0.f, 0.f, 0.f));

  //get images width, height
  const std::size_t width = images.front().Width();
  const std::size_t height = images.front().Height();

  const float maxLum = 1000.0;
  const float minLum = 0.0001;

  #pragma omp parallel for
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      //for each pixels
      image::RGBfColor &radianceColor = radiance(y, x);

      for(std::size_t channel = 0; channel < 3; ++channel)
      {
        double wsum = 0.0;
        double wdiv = 0.0;
        double lowValue = images.at(0)(y, x)(channel);
        double highValue = images.at(images.size()-1)(y, x)(channel);


        for(std::size_t i = 0; i < images.size(); ++i)
        {
          //for each images
          const double value = images[i](y, x)(channel);
          const double time = times[i];
          double w = std::max(0.f, weight(value, channel) - weight(0.05, 0));

          const double r = response(value, channel);

          wsum += w * r / time;
          wdiv += w;
        }

        double clampedHighValue = 1.0 - (1.0 / (1.0 + expf(10.0 * ((highValue - 0.9) / 0.2))));
        double clampedLowValue = 1.0 / (1.0 + expf(10.0 * ((lowValue - 0.005) / 0.01)));


        if(!robCalibrate && clampedValueCorrection != 0.f)
        {
          radianceColor(channel) = (1.0 - clampedHighValue - clampedLowValue) * wsum / std::max(0.001, wdiv) * targetTime + clampedHighValue * maxLum * clampedValueCorrection + clampedLowValue * minLum * clampedValueCorrection;
        }
        else
        {
          radianceColor(channel) = wsum / std::max(0.001, wdiv) * targetTime;
        }
      }
    }
  }
}


} // namespace hdr
} // namespace aliceVision
