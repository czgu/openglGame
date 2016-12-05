#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
	JointNode(const std::string & name);
	virtual ~JointNode();

	void set_joint_x(double min, double init, double max);
	void set_joint_y(double min, double init, double max);

	struct JointRange {
		double min, init, max, curr;
	};

    void rotate_joint(JointRange& range, double delta);
    void reset_joint();

    void set_x_rotate(double delta);

    glm::mat4 get_x_rotate() const;
    glm::mat4 get_y_rotate() const;


	JointRange m_joint_x, m_joint_y;
    bool hasEmptyJoint;
};
