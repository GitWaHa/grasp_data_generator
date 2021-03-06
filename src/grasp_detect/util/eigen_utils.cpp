#include "grasp_detect/util/eigen_utils.h"

namespace grasp_detect
{
	namespace util
	{

		Eigen::Matrix3Xd EigenUtils::sliceMatrix(const Eigen::Matrix3Xd &mat,
												 const std::vector<int> &indices)
		{
			Eigen::Matrix3Xd mat_out(3, indices.size());

			for (int j = 0; j < indices.size(); j++)
			{
				mat_out.col(j) = mat.col(indices[j]);
			}

			return mat_out;
		}

		Eigen::MatrixXi EigenUtils::sliceMatrix(const Eigen::MatrixXi &mat,
												const std::vector<int> &indices)
		{
			Eigen::MatrixXi mat_out(mat.rows(), indices.size());

			for (int j = 0; j < indices.size(); j++)
			{
				mat_out.col(j) = mat.col(indices[j]);
			}

			return mat_out;
		}

		Eigen::Vector3i EigenUtils::floorVector(const Eigen::Vector3f &a)
		{
			Eigen::Vector3i b;
			b << floor(a(0)), floor(a(1)), floor(a(2));
			return b;
		}

	} // namespace util
} // namespace grasp_detect
