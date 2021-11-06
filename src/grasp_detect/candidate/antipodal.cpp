#include "grasp_detect/candidate/antipodal.h"

namespace grasp_detect
{
	namespace candidate
	{

		const int Antipodal::NO_GRASP = 0;	 // normals point not toward any finger
		const int Antipodal::HALF_GRASP = 1; // normals point towards one finger
		const int Antipodal::FULL_GRASP = 2; // normals point towards both fingers

		int Antipodal::evaluateGrasp(const util::PointList &point_list,
									 double extremal_thresh, int lateral_axis,
									 int forward_axis, int vertical_axis, double &score) const
		{
			int result = NO_GRASP;

			const Eigen::Matrix3Xd &pts = point_list.getPoints();
			const Eigen::Matrix3Xd &normals = point_list.getNormals();

			// Select points that are extremal and have their surface normal within the
			// friction cone of the closing direction.
			Eigen::Vector3d l, r;
			if (lateral_axis == 0)
			{
				l << -1.0, 0.0, 0.0;
				r << 1.0, 0.0, 0.0;
			}
			else if (lateral_axis == 1)
			{
				l << 0.0, -1.0, 0.0;
				r << 0.0, 1.0, 0.0;
			}

			double min_x = pts.row(lateral_axis).minCoeff() + extremal_thresh;
			double max_x = pts.row(lateral_axis).maxCoeff() - extremal_thresh;
			std::vector<int> left_idx_viable, right_idx_viable;

			// 统计左右接触点
			for (int i = 0; i < pts.cols(); i++)
			{
				if (pts(lateral_axis, i) < min_x)
					left_idx_viable.push_back(i);
				if (pts(lateral_axis, i) > max_x)
					right_idx_viable.push_back(i);
			}

			// 获取左右接触点
			Eigen::Matrix3Xd left_pts_viable(3, left_idx_viable.size());
			Eigen::Matrix3Xd right_pts_viable(3, right_idx_viable.size());
			for (int i = 0; i < left_idx_viable.size(); i++)
			{
				left_pts_viable.col(i) = pts.col(left_idx_viable[i]);
			}
			for (int i = 0; i < right_idx_viable.size(); i++)
			{
				right_pts_viable.col(i) = pts.col(right_idx_viable[i]);
			}

			// 求接触点集合的中心点坐标
			auto left_center = left_pts_viable.rowwise().mean();
			auto right_center = right_pts_viable.rowwise().mean();
			Eigen::Vector3d left_center_vec(left_center(0), left_center(1), left_center(2));
			Eigen::Vector3d right_center_vec(right_center(0), right_center(1), right_center(2));

			// 右----->左
			Eigen::Vector3d left_right_vector = left_center_vec - right_center_vec;
			left_right_vector.normalize();

			// std::cout << "center: " << left_center << "," << right_center << "," << left_right_vector << std::endl;
			int max_angle_friction = 90;
			double cos_friction_coeff_ = cos(friction_coeff_ * M_PI / 180.0);
			double l_cos_friction = 0.1;
			while (l_cos_friction <= max_angle_friction)
			{
				int left_close_count = 0;
				for (int i : left_idx_viable)
				{
					if (acos(std::abs(left_right_vector.transpose() * normals.col(i))) <= (l_cos_friction * M_PI / 180.0))
						left_close_count++;
				}
				if (left_close_count >= min_viable_)
					break;
				l_cos_friction += 2;
			}
			// std::cout << "l_cos_friction: " << l_cos_friction;
			l_cos_friction = 1.0 - l_cos_friction / 100;
			// std::cout << " l_cos_friction: " << l_cos_friction << std::endl;
			double r_cos_friction = 0.1;
			while (r_cos_friction <= max_angle_friction)
			{
				int right_close_count = 0;
				for (int i : right_idx_viable)
				{
					if (acos(std::abs(-1 * left_right_vector.transpose() * normals.col(i))) <= (r_cos_friction * M_PI / 180.0))
						right_close_count++;
				}
				if (right_close_count >= min_viable_)
					break;
				r_cos_friction += 2;
			}
			// std::cout << "r_cos_friction: " << r_cos_friction;
			r_cos_friction = 1.0 - r_cos_friction / 100;
			// std::cout << "r_cos_friction: " << r_cos_friction << std::endl;

			double left_right_cos_friction = acos(std::abs(left_right_vector.transpose() * l)) * 180 / M_PI;
			// std::cout << "left_right_cos_friction: " << left_right_cos_friction;
			left_right_cos_friction = 1.0 - left_right_cos_friction / 100;
			// std::cout << "left_right_cos_friction: " << left_right_cos_friction << std::endl;

			score = std::min(r_cos_friction, l_cos_friction);
			score = std::min(score, left_right_cos_friction);

			// std::cout << "score: " << score << std::endl;

			return result;
		} // namespace candidate

		int Antipodal::evaluateGrasp(const util::PointList &point_list,
									 double extremal_thresh, int lateral_axis,
									 int forward_axis, int vertical_axis) const
		{
			int result = NO_GRASP;

			const Eigen::Matrix3Xd &pts = point_list.getPoints();
			const Eigen::Matrix3Xd &normals = point_list.getNormals();

			// Select points that are extremal and have their surface normal within the
			// friction cone of the closing direction.
			Eigen::Vector3d l, r;
			if (lateral_axis == 0)
			{
				l << -1.0, 0.0, 0.0;
				r << 1.0, 0.0, 0.0;
			}
			else if (lateral_axis == 1)
			{
				l << 0.0, -1.0, 0.0;
				r << 0.0, 1.0, 0.0;
			}
			double cos_friction_coeff_ = cos(friction_coeff_ * M_PI / 180.0);
			double min_x = pts.row(lateral_axis).minCoeff() + extremal_thresh;
			double max_x = pts.row(lateral_axis).maxCoeff() - extremal_thresh;
			std::vector<int> left_idx_viable, right_idx_viable;

			for (int i = 0; i < pts.cols(); i++)
			{
				bool is_within_left_close =
					(l.transpose() * normals.col(i)) > cos_friction_coeff_;
				bool is_within_right_close =
					(r.transpose() * normals.col(i)) > cos_friction_coeff_;
				bool is_left_extremal = pts(lateral_axis, i) < min_x;
				bool is_right_extremal = pts(lateral_axis, i) > max_x;

				if (is_within_left_close && is_left_extremal)
					left_idx_viable.push_back(i);
				if (is_within_right_close && is_right_extremal)
					right_idx_viable.push_back(i);
			}

			if (left_idx_viable.size() > 0 || right_idx_viable.size() > 0)
				result = HALF_GRASP;

			if (left_idx_viable.size() > 0 && right_idx_viable.size() > 0)
			{
				Eigen::Matrix3Xd left_pts_viable(3, left_idx_viable.size());
				Eigen::Matrix3Xd right_pts_viable(3, right_idx_viable.size());

				for (int i = 0; i < left_idx_viable.size(); i++)
				{
					left_pts_viable.col(i) = pts.col(left_idx_viable[i]);
				}
				for (int i = 0; i < right_idx_viable.size(); i++)
				{
					right_pts_viable.col(i) = pts.col(right_idx_viable[i]);
				}

				double top_viable_y = std::min(left_pts_viable.row(forward_axis).maxCoeff(),
											   right_pts_viable.row(forward_axis).maxCoeff());
				double bottom_viable_y = std::max(left_pts_viable.row(forward_axis).minCoeff(),
												  right_pts_viable.row(forward_axis).minCoeff());

				double top_viable_z = std::min(left_pts_viable.row(vertical_axis).maxCoeff(),
											   right_pts_viable.row(vertical_axis).maxCoeff());
				double bottom_viable_z = std::max(left_pts_viable.row(vertical_axis).minCoeff(),
												  right_pts_viable.row(vertical_axis).minCoeff());

				int num_viable_left = 0;
				for (int i = 0; i < left_pts_viable.cols(); i++)
				{
					double y = left_pts_viable(forward_axis, i);
					double z = left_pts_viable(vertical_axis, i);
					if (y >= bottom_viable_y && y <= top_viable_y && z >= bottom_viable_z &&
						z <= top_viable_z)
						num_viable_left++;
				}
				int num_viable_right = 0;
				for (int i = 0; i < right_pts_viable.cols(); i++)
				{
					double y = right_pts_viable(forward_axis, i);
					double z = right_pts_viable(vertical_axis, i);
					if (y >= bottom_viable_y && y <= top_viable_y && z >= bottom_viable_z &&
						z <= top_viable_z)
						num_viable_right++;
				}

				if (num_viable_left >= min_viable_ && num_viable_right >= min_viable_)
				{
					result = FULL_GRASP;
				}
				// result = FULL_GRASP;
			}

			return result;
		}

		int Antipodal::evaluateGrasp(const Eigen::Matrix3Xd &normals,
									 double thresh_half, double thresh_full) const
		{
			int num_thresh = 6;
			int grasp = 0;
			double cos_thresh = cos(thresh_half * M_PI / 180.0);
			int numl = 0;
			int numr = 0;
			Eigen::Vector3d l, r;
			l << -1, 0, 0;
			r << 1, 0, 0;
			bool is_half_grasp = false;
			bool is_full_grasp = false;

			// check whether this is a half grasp
			for (int i = 0; i < normals.cols(); i++)
			{
				if (l.dot(normals.col(i)) > cos_thresh)
				{
					numl++;
					if (numl > num_thresh)
					{
						is_half_grasp = true;
						break;
					}
				}

				if (r.dot(normals.col(i)) > cos_thresh)
				{
					numr++;
					if (numr > num_thresh)
					{
						is_half_grasp = true;
						break;
					}
				}
			}

			// check whether this is a full grasp
			cos_thresh = cos(thresh_full * M_PI / 180.0);
			numl = 0;
			numr = 0;

			for (int i = 0; i < normals.cols(); i++)
			{
				if (l.dot(normals.col(i)) > cos_thresh)
				{
					numl++;
					if (numl > num_thresh && numr > num_thresh)
					{
						is_full_grasp = true;
						break;
					}
				}

				if (r.dot(normals.col(i)) > cos_thresh)
				{
					numr++;
					if (numl > num_thresh && numr > num_thresh)
					{
						is_full_grasp = true;
						break;
					}
				}
			}

			if (is_full_grasp)
				return FULL_GRASP;
			else if (is_half_grasp)
				return HALF_GRASP;

			return NO_GRASP;
		}

	} // namespace candidate
} // namespace grasp_detect