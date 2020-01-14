// This file is part of the AliceVision project.
// Copyright (c) 2016 AliceVision contributors.
// Copyright (c) 2012 openMVG contributors.
// Copyright (c) 2010 libmv contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include "rotationKernelSolver.hpp"
#include <iostream>

#include <Eigen/SVD>

namespace aliceVision {
namespace rotation {
namespace kernel {

void ThreePointRotationSolver::Solve(const Mat &p1, const Mat &p2, vector<Mat3> *Rs) {

  assert(3 == p1.rows());
  assert(3 <= p2.cols());
  assert(p1.rows() == p2.rows());
  assert(p1.cols() == p2.cols());

  Mat::Index n = p1.cols();

  Eigen::Matrix3d M = Eigen::Matrix3d::Zero();

  for (int i = 0; i < n; i++) {
    
    for (int k = 0; k < 3; k++) {
      for (int l = 0; l < 3; l++) {

        M(k, l) += p1(k, i) * p2(l, i);
      }
    }
  }

  Eigen::JacobiSVD<Eigen::Matrix3d> svd(M, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::Matrix3d U = svd.matrixU();
  Eigen::Matrix3d V = svd.matrixV();

  Eigen::Matrix3d G = U * V.transpose();
  Eigen::Matrix3d D = Einge::Matrix3d::Identity();
  D(2, 2) = 1.0 / G.determinant();

  Eigen::Matrix3d R = U * D * V.transpose();

  Rs->push_back(R);
}

}  // namespace kernel
}  // namespace rotation
}  // namespace aliceVision
