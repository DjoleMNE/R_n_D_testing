#include <kdl/chainidsolver_vereshchagin.hpp>
#include <kdl/framevel.hpp>
#include <kdl/frames_io.hpp>
#include <kdl/framevel_io.hpp>
#include <kdl/kinfam_io.hpp>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

using namespace KDL;
KDL::Chain chaindyn;

void setUp()
{
    srand((unsigned)time( NULL ));

    //chain definition for vereshchagin's dynamic solver
    //last 3 inputs???? offset of the joint????..probably yes
    Joint rotJoint0 = Joint(Joint::RotZ, 1, 0, 0.01);
    Joint rotJoint1 = Joint(Joint::RotZ, 1, 0, 0.01);

    // RPY(roll,pitch,yaw) Rotation built from Roll-Pitch-Yaw angles
    Frame refFrame(Rotation::RPY(0.0, 0.0, 0.0), Vector(0.0, 0.0, 0.0));
    Frame frame1(Rotation::RPY(0.0, 0.0, 0.0), Vector(0.0, -0.4, 0.0));
    Frame frame2(Rotation::RPY(0.0, 0.0, 0.0), Vector(0.0, -0.4, 0.0));

    //chain segments
    //Frames desctibe pose of the segment tip, wrt joint frame
    Segment segment1 = Segment(rotJoint0, frame1);
    Segment segment2 = Segment(rotJoint1, frame2);

    //rotational inertia around symmetry axis of rotation
    // why all zeros?????
    RotationalInertia rotInerSeg1(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    //spatial inertia
    // center of mass at the same position as tip  of segment???
    RigidBodyInertia inerSegment1(0.3, Vector(0.0, -0.4, 0.0), rotInerSeg1);
    RigidBodyInertia inerSegment2(0.3, Vector(0.0, -0.4, 0.0), rotInerSeg1);
    segment1.setInertia(inerSegment1);
    segment2.setInertia(inerSegment2);

    //chain
    chaindyn.addSegment(segment1);
    chaindyn.addSegment(segment2);

	// // Motoman SIA10 Chain (for IK singular value tests)
	// motomansia10.addSegment(Segment(Joint(Joint::None),
	// 								Frame::DH_Craig1989(0.0, 0.0, 0.36, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame::DH_Craig1989(0.0, M_PI_2, 0.0, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame::DH_Craig1989(0.0, -M_PI_2, 0.36, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame::DH_Craig1989(0.0, M_PI_2, 0.0, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame::DH_Craig1989(0.0, -M_PI_2, 0.36, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame::DH_Craig1989(0.0, M_PI_2, 0.0, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame::DH_Craig1989(0.0, -M_PI_2, 0.0, 0.0)));
	// motomansia10.addSegment(Segment(Joint(Joint::RotZ),
	// 								Frame(Rotation::Identity(),Vector(0.0,0.0,0.155))));
}

	void VereshchaginTest(double torque_1, double torque_2)
	{
        // std::cout << torque_1 << '\n';
        ofstream myfile;
        myfile.open ("/home/djole/Downloads/Master/R_&_D/KDL_GIT/Testing_repo/src/joint_poses.txt");
        //~Definition of constraints and external disturbances
        //-------------------------------------------------------------------------------------//
	    Vector constrainXLinear(1.0, 0.0, 0.0);
        // Vector constrainXLinear(0.0, 0.0, 0.0);
	    Vector constrainXAngular(0.0, 0.0, 0.0);
	    Vector constrainYLinear(0.0, 0.0, 0.0);
	    Vector constrainYAngular(0.0, 0.0, 0.0);
	    //Vector constrainZLinear(0.0, 0.0, 0.0);
	    //Vector constrainZAngular(0.0, 0.0, 0.0);
	    Twist constraintForcesX(constrainXLinear, constrainXAngular);
	    Twist constraintForcesY(constrainYLinear, constrainYAngular);
	    //Twist constraintForcesZ(constrainZLinear, constrainZAngular);
	    Jacobian alpha(1);

	    //End effector is not allowed to move in x direction linearly????
	    alpha.setColumn(0, constraintForcesX);
	    //alpha.setColumn(0, constraintForcesZ);

	    //Acceleration energy at  the end-effector
	    JntArray betha(1); //set to zero....no desired acceleration???
        //task not specified? or the task is to keep ee_acc to 0???
	    betha(0) = 0.0;
	    //betha(1) = 0.0;
	    //betha(2) = 0.0;

	    //arm root acceleration
	    Vector linearAcc(0.0, 9.81, 0.0); //gravitational acceleration along Y
	    Vector angularAcc(0.0, 0.0, 0.0);
	    Twist twist1(linearAcc, angularAcc);

	    //external forces on the arm
	    Vector externalForce1(0.0, 0.0, 0.0);
	    Vector externalTorque1(0.0, 0.0, 0.0);
	    Vector externalForce2(0.0, 0.0, 0.0);
	    Vector externalTorque2(0.0, 0.0, 0.0);
	    Wrench externalNetForce1(externalForce1, externalTorque1);
	    Wrench externalNetForce2(externalForce2, externalTorque2);
	    Wrenches externalNetForce;
	    externalNetForce.push_back(externalNetForce1);
	    externalNetForce.push_back(externalNetForce2);


	    //Definition of solver and initial configuration
	    //-------------------------------------------------------------------------------------//
        int numberOfConstraints =1;
        // int numberOfConstraints = 0;
	    ChainIdSolver_Vereshchagin constraintSolver(chaindyn, twist1, numberOfConstraints);

	    //These arrays of joint values contain actual and desired values
	    //actual is generated by a solver and integrator
	    //desired is given by an interpolator
	    //error is the difference between desired-actual
	    //in this test only the actual values are printed.
	    const int k = 1;
	    JntArray jointPoses[k];
	    JntArray jointRates[k];
	    JntArray jointAccelerations[k];
	    JntArray jointTorques[k];
	    for (int i = 0; i < k; i++)
	    {
	        JntArray jointValues(chaindyn.getNrOfJoints());
	        jointPoses[i] = jointValues;
	        jointRates[i] = jointValues;
	        jointAccelerations[i] = jointValues;
	        jointTorques[i] = jointValues;
	    }

	    // Initial arm position configuration/constraint
	    JntArray jointInitialPose(chaindyn.getNrOfJoints());
	    jointInitialPose(0) = 0.0; // initial joint0 pose
	    jointInitialPose(1) = M_PI/6.0; //initial joint1 pose, negative in clockwise...180deg
	    //j0=0.0, j1=pi/6.0 correspond to x = 0.2, y = -0.7464
	    //j0=2*pi/3.0, j1=pi/4.0 correspond to x = 0.44992, y = 0.58636

	    //actual
	    jointPoses[0](0) = jointInitialPose(0);
	    jointPoses[0](1) = jointInitialPose(1);

	    jointTorques[0](0) = torque_1;
	    jointTorques[0](1) = torque_2;

	    //Definition of process main loop
	    //-------------------------------------------------------------------------------------//
	    //Time required to complete the task move(frameinitialPose, framefinalPose)
	    double taskTimeConstant = 0.1;
	    double simulationTime = 1*taskTimeConstant;
	    double timeDelta = 0.01;
	    int status;

	    const std::string msg = "Assertion failed, check matrix and array sizes";
	    std::cout << "Input to algorithm" << '\n';
	    std::cout << " " << '\n';
	    printf("%s          %s      %s         %s     %s       %s         %s      %s      %s\n", "Time for Task  Constant", "j0_pose", "j1_pose", "j0_rate", "j1_rate", "j0_acc", "j1_acc", "j0_torque", "j1_torque");
	    printf("%f                        %f      %f       %f     %f       %f      %f     %f      %f\n", taskTimeConstant, jointPoses[0](0), jointPoses[0](1), jointRates[0](0), jointRates[0](1), jointAccelerations[0](0), jointAccelerations[0](1), jointTorques[0](0), jointTorques[0](1));
	    std::cout << " " << '\n';
	    std::cout << "Output of algorithm: " << '\n';
	    std::cout << " " << '\n';
	    printf("time              j0_pose       j1_pose        j0_rate       j1_rate         j0_acc        j1_acc       j0_constraintTau         j1_constraintTau \n");

	    for (double t = 0.0; t <=simulationTime; t = t + timeDelta)
	    {
			int return_solver = constraintSolver.CartToJnt(jointPoses[0], jointRates[0], jointAccelerations[0], alpha, betha, externalNetForce, jointTorques[0]);

			// if return...
	        //Integration(robot joint values for rates and poses; actual) at the given "instanteneous" interval for joint position and velocity.
	        jointRates[0](0) = jointRates[0](0) + jointAccelerations[0](0) * timeDelta; //Euler Forward
	        jointPoses[0](0) = jointPoses[0](0) + (jointRates[0](0) - jointAccelerations[0](0) * timeDelta / 2.0) * timeDelta; //Trapezoidal rule
	        jointRates[0](1) = jointRates[0](1) + jointAccelerations[0](1) * timeDelta; //Euler Forward
	        jointPoses[0](1) = jointPoses[0](1) + (jointRates[0](1) - jointAccelerations[0](1) * timeDelta / 2.0) * timeDelta;
            myfile <<jointPoses[0](0)<<" "<<jointPoses[0](1)<<"\n";
            // myfile <<jointPoses[0](0)<<" "<<jointPoses[0](1)<<" "<<jointRates[0](0)<<" "<<jointRates[0](1)<<" "<<jointAccelerations[0](0)<<" "<<jointAccelerations[0](1)<<" "<<jointTorques[0](0)<<" "<<jointTorques[0](1)<<"\n";
	        printf("%f          %f      %f       %f     %f       %f      %f     %f                 %f\n", t, jointPoses[0](0), jointPoses[0](1), jointRates[0](0), jointRates[0](1), jointAccelerations[0](0), jointAccelerations[0](1), jointTorques[0](0), jointTorques[0](1));
	    }
         myfile.close();
	}



int main(int argc, char* argv[])
{
    if (argc < 2) {
        // Tell the user how to run the program
        std::cerr << "Usage: " << argv[0] << " Torgue_1 Torque_2" << std::endl;
        /* "Usage messages" are a conventional way of telling the user
         * how to run a program if they enter the command incorrectly.
         */
        return 1;
    }
    // Print the user's name:
    // std::cout << argv[0] << "says hello, " << argv[1] << "!" << std::endl;
	setUp();
	VereshchaginTest(atof (argv[1]),atof (argv[2]));
	return 0;
}
