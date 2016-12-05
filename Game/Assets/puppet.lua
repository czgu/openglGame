-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
-- rootnode:scale( 0.25, 0.25, 0.25 )

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
purple = gr.material({1.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)

function makeLimb(isUpper)
    limbLen = 0.3
    if isUpper then
        shoulderJoint = gr.joint('shoulderJoint', {180, 360, 360}, {0, 0, 0})
    else
        limbLen = 0.5
        shoulderJoint = gr.joint('thighJoint', {-90, 0, 60}, {0, 0, 0})
    end

    shoulder = gr.mesh('sphere', 'shoulder')
    shoulder:scale(0.05, 0.05, 0.05)
    shoulder:set_material(blue)
    shoulderJoint:add_child(shoulder)

    upper = gr.mesh('cube', 'UpperLimb')
    upper:scale(0.1, limbLen, 0.1)
    upper:translate(0, -limbLen/2, 0)
    upper:set_material(red)
    shoulderJoint:add_child(upper)

    if isUpper then
        elbowJoint = gr.joint('elbowJoint', {-160, -80, 0}, {0, 0, 0})
    else
        elbowJoint = gr.joint('kneeJoint', {0, 0, 120}, {0, 0, 0})
    end

    elbowJoint:translate(0, -limbLen, 0);

    elbow= gr.mesh('sphere', 'elbow')
    elbow:scale(0.07, 0.07, 0.07)
    elbow:set_material(blue)
    elbowJoint:add_child(elbow)

    lower = gr.mesh('cube', 'LowerLimb')
    lower:scale(0.1, limbLen, 0.1)
    lower:translate(0, -limbLen/2, 0);
    lower:set_material(green)
    elbowJoint:add_child(lower)

    shoulderJoint:add_child(elbowJoint)
    return shoulderJoint
end

function makeShoulder(isUpper)
    centreJoint = gr.joint('centreJoint', {0, 0, 0}, {-30, 0, 30});

    shoulder = gr.mesh('sphere', 'Shoulder');
    shoulder:scale(0.3, 0.07, 0.2);
    shoulder:set_material(yellow);
    centreJoint:add_child(shoulder)

    leftArm = makeLimb(isUpper)
    leftArm:translate(-0.3, 0.0, 0.0)
    centreJoint:add_child(leftArm)

    rightArm = makeLimb(isUpper)
    rightArm:translate(0.3, 0.0, 0.0)
    centreJoint:add_child(rightArm)

    return centreJoint
end

function makeHead()
    neckLowJoint = gr.joint('neckLowJoint', {0, 0, 90}, {-90, 0, 90});

    neck = gr.mesh('cube', 'neck')
    neck:scale(0.10, 0.1, 0.10)
    neck:translate(0.0, 0.05, 0.0)
    neck:set_material(blue)
    neckLowJoint:add_child(neck)

    head = gr.mesh('cube', 'head')
    neckLowJoint:add_child(head)
    head:scale(0.25, 0.25, 0.25)
    head:translate(0.0, 0.225, 0.0)
    head:set_material(white)

    ears = gr.mesh('sphere', 'ears')
    head:add_child(ears)
    ears:scale(1.6, 0.08, 0.08)
    ears:set_material(red)
    ears:set_material(blue)

    leftEye = gr.mesh('sphere', 'leftEye')
    head:add_child(leftEye)
    leftEye:scale(0.1, 0.1, 0.1)
    leftEye:translate(-0.25, 0.15, 0.5)
    leftEye:set_material(blue)

    rightEye = gr.mesh('sphere', 'rightEye')
    head:add_child(rightEye)
    rightEye:scale(0.1, 0.1, 0.1)
    rightEye:translate(0.25, 0.15, 0.5)
    rightEye:set_material(blue)

    return neckLowJoint
end

jointTorso = gr.joint('jointTorso', {0,0,0}, {0,0,0});
rootnode:add_child(jointTorso)

shoulder = makeShoulder(true)
shoulder:translate(0, 1.45, 0);
jointTorso:add_child(shoulder)

hip = makeShoulder(false);
hip:translate(0, 1.0, 0);
jointTorso:add_child(hip)

head = makeHead();
head:translate(0, 1.50, 0)
jointTorso:add_child(head)

torso = gr.mesh('cube', 'torso')
torso:set_material(white)
torso:scale(0.30,0.45,0.2);
torso:translate(0, 1.225, 0);
jointTorso:add_child(torso)

return rootnode
