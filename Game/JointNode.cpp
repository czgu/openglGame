#include "JointNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name), hasEmptyJoint(false)
{
	m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;
	m_joint_x.curr = init;

    if (max - min == 0) {
        hasEmptyJoint = true;
    }
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;
	m_joint_y.curr = init;

    if (max - min == 0) {
        hasEmptyJoint = true;
    }
}

static double clamp(JointNode::JointRange range, double val) {
    if (val > range.max) {
        return range.max;
    } else if (val < range.min) {
        return range.min;
    }
    return val;
}

void JointNode::set_x_rotate(double delta) {
    m_joint_x.curr = clamp(m_joint_x, delta);
}

void JointNode::rotate_joint(JointRange& range, double delta) {
    range.curr = clamp(range, delta + range.curr);
}

void JointNode::reset_joint() {
    m_joint_x.curr = m_joint_x.init;
    m_joint_y.curr = m_joint_y.init;
    isSelected = false;
}

glm::mat4 JointNode::get_x_rotate() const {
    return glm::rotate((float)degreesToRadians(m_joint_x.curr), glm::vec3(1, 0, 0));
}

glm::mat4 JointNode::get_y_rotate() const {
    return glm::rotate((float)degreesToRadians(m_joint_y.curr), glm::vec3(0, 1, 0));
}

