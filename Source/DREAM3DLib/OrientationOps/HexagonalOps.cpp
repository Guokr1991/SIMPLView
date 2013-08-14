/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Groeber (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "HexagonalOps.h"
// Include this FIRST because there is a needed define for some compiles
// to expose some of the constants needed below
#include "DREAM3DLib/Common/DREAM3DMath.h"
#include "DREAM3DLib/Math/OrientationMath.h"
#include "DREAM3DLib/Common/ModifiedLambertProjection.h"
#include "DREAM3DLib/Utilities/ImageUtilities.h"


namespace Detail {

  static const float HexDim1InitValue = powf((0.75f*((float(DREAM3D::Constants::k_Pi)/2.0f)-sinf((float(DREAM3D::Constants::k_Pi)/2.0f)))),(1.0f/3.0f));
  static const float HexDim2InitValue = powf((0.75f*((float(DREAM3D::Constants::k_Pi)/2.0f)-sinf((float(DREAM3D::Constants::k_Pi)/2.0f)))),(1.0f/3.0f));
  static const float HexDim3InitValue = powf((0.75f*((float(DREAM3D::Constants::k_Pi)/6.0f)-sinf((float(DREAM3D::Constants::k_Pi)/6.0f)))),(1.0f/3.0f));
  static const float HexDim1StepValue = HexDim1InitValue/18.0f;
  static const float HexDim2StepValue = HexDim2InitValue/18.0f;
  static const float HexDim3StepValue = HexDim3InitValue/6.0f;
}

static const QuatF HexQuatSym[12] = {QuaternionMathF::New(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f),
                                     QuaternionMathF::New(0.000000000f, 0.000000000f, 0.500000000f, 0.866025400f),
                                     QuaternionMathF::New(0.000000000f, 0.000000000f, 0.866025400f, 0.500000000f),
                                     QuaternionMathF::New(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f),
                                     QuaternionMathF::New(0.000000000f, 0.000000000f, 0.866025400f, -0.50000000f),
                                     QuaternionMathF::New(0.000000000f, 0.000000000f, 0.500000000f, -0.86602540f),
                                     QuaternionMathF::New(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f),
                                     QuaternionMathF::New(0.866025400f, 0.500000000f, 0.000000000f, 0.000000000f),
                                     QuaternionMathF::New(0.500000000f, 0.866025400f, 0.000000000f, 0.000000000f),
                                     QuaternionMathF::New(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f),
                                     QuaternionMathF::New(-0.50000000f, 0.866025400f, 0.000000000f, 0.000000000f),
                                     QuaternionMathF::New(-0.86602540f, 0.500000000f, 0.000000000f, 0.000000000)};


static const float HexRodSym[12][3] = {{0.0f, 0.0f, 0.0f},
                                       {0.0f, 0.0f, 0.57735f},
                                       {0.0f, 0.0f, 1.73205f},
                                       {0.0f, 0.0f, 1000000000000.0f},
                                       {0.0f, 0.0f, -1.73205f},
                                       {0.0f, 0.0f, -0.57735f},
                                       {1000000000000.0f, 0.0f, 0.0f},
                                       {8660254000000.0f, 5000000000000.0f, 0.0f},
                                       {5000000000000.0f, 8660254000000.0f, 0.0f},
                                       {0.0f, 1000000000000.0f, 0.0f},
                                       {-5000000000000.0f, 8660254000000.0f, 0.0f},
                                       {-8660254000000.0f, 5000000000000.0f, 0.0f}};
static const float HexMatSym[12][3][3] =
{{{1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, 1.0}},

 {{-0.5, DREAM3D::Constants::k_Root3Over2,  0.0},
  {-DREAM3D::Constants::k_Root3Over2, -0.5, 0.0},
  {0.0, 0.0,  1.0}},

 {{-0.5, -DREAM3D::Constants::k_Root3Over2,  0.0},
  {DREAM3D::Constants::k_Root3Over2, -0.5, 0.0},
  {0.0, 0.0,  1.0}},

 {{0.5, DREAM3D::Constants::k_Root3Over2,  0.0},
  {-DREAM3D::Constants::k_Root3Over2, 0.5, 0.0},
  {0.0, 0.0,  1.0}},

 {{-1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {0.0, 0.0, 1.0}},

 {{0.5, -DREAM3D::Constants::k_Root3Over2,  0.0},
  {DREAM3D::Constants::k_Root3Over2, 0.5, 0.0},
  {0.0, 0.0,  1.0}},

 {{-0.5, -DREAM3D::Constants::k_Root3Over2,  0.0},
  {-DREAM3D::Constants::k_Root3Over2, 0.5, 0.0},
  {0.0, 0.0,  -1.0}},

 {{1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0},
  {0.0, 0.0, -1.0}},

 {{-0.5, DREAM3D::Constants::k_Root3Over2,  0.0},
  {DREAM3D::Constants::k_Root3Over2, 0.5, 0.0},
  {0.0, 0.0,  -1.0}},

 {{0.5, DREAM3D::Constants::k_Root3Over2,  0.0},
  {DREAM3D::Constants::k_Root3Over2, -0.5, 0.0},
  {0.0, 0.0,  -1.0}},

 {{-1.0, 0.0, 0.0},
  {0.0, 1.0, 0.0},
  {0.0, 0.0, -1.0}},

 {{0.5, -DREAM3D::Constants::k_Root3Over2,  0.0},
  {-DREAM3D::Constants::k_Root3Over2, -0.5, 0.0},
  {0.0, 0.0,  -1.0}}};


using namespace Detail;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
HexagonalOps::HexagonalOps()
{
  float junk1 =  HexDim1StepValue * 1.0f;
  float junk2 = junk1/HexDim2StepValue;
  float junk3 = junk2/HexDim3StepValue;
  junk1 = junk3/junk2;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
HexagonalOps::~HexagonalOps()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
float HexagonalOps::_calcMisoQuat(const QuatF quatsym[12], int numsym,
QuatF &q1, QuatF &q2,
float &n1, float &n2, float &n3)
{
  float wmin = 9999999.0f; //,na,nb,nc;
  float w = 0;
  float n1min = 0.0f;
  float n2min = 0.0f;
  float n3min = 0.0f;
  QuatF qr;
  QuatF qc;
  QuatF q2inv;
  QuaternionMathF::Copy(q2, q2inv);
  QuaternionMathF::Conjugate(q2inv);

  QuaternionMathF::Multiply(q2inv, q1, qr);
  for (int i = 0; i < numsym; i++)
  {
    QuaternionMathF::Multiply(qr, quatsym[i], qc);
    if (qc.w < -1) {
      qc.w = -1;
    }
    else if (qc.w > 1) {
      qc.w = 1;
    }

    OrientationMath::QuattoAxisAngle(qc, w, n1, n2, n3);

    if (w > DREAM3D::Constants::k_Pi) {
      w = DREAM3D::Constants::k_2Pi - w;
    }
    if (w < wmin)
    {
      wmin = w;
      n1min = n1;
      n2min = n2;
      n3min = n3;
    }
  }
  float denom = sqrt((n1min*n1min+n2min*n2min+n3min*n3min));
  n1 = n1min/denom;
  n2 = n2min/denom;
  n3 = n3min/denom;
  if(denom == 0) n1 = 0.0, n2 = 0.0, n3 = 1.0;
  if(wmin == 0) n1 = 0.0, n2 = 0.0, n3 = 1.0;
  return wmin;
}

float HexagonalOps::getMisoQuat(QuatF &q1, QuatF &q2, float &n1, float &n2, float &n3)
{
  int numsym = 12;

  return _calcMisoQuat(HexQuatSym, numsym, q1, q2, n1, n2, n3);
}

void HexagonalOps::getQuatSymOp(int i, QuatF &q)
{
  QuaternionMathF::Copy(HexQuatSym[i], q);
  //  q.x = HexQuatSym[i][0];
  //  q.y = HexQuatSym[i][1];
  //  q.z = HexQuatSym[i][2];
  //  q.w = HexQuatSym[i][3];
}

void HexagonalOps::getRodSymOp(int i,float *r)
{
  r[0] = HexRodSym[i][0];
  r[1] = HexRodSym[i][1];
  r[2] = HexRodSym[i][2];
}

void HexagonalOps::getMatSymOp(int i,float g[3][3])
{
  g[0][0] = HexMatSym[i][0][0];
  g[0][1] = HexMatSym[i][0][1];
  g[0][2] = HexMatSym[i][0][2];
  g[1][0] = HexMatSym[i][1][0];
  g[1][1] = HexMatSym[i][1][1];
  g[1][2] = HexMatSym[i][1][2];
  g[2][0] = HexMatSym[i][2][0];
  g[2][1] = HexMatSym[i][2][1];
  g[2][2] = HexMatSym[i][2][2];
}

void HexagonalOps::getODFFZRod(float &r1,float &r2, float &r3)
{
  int numsym = 12;

  _calcRodNearestOrigin(HexRodSym, numsym, r1, r2, r3);
}

void HexagonalOps::getMDFFZRod(float &r1,float &r2, float &r3)
{
  float w, n1, n2, n3;
  float FZn1, FZn2, FZn3;
  float n1n2mag;

  _calcRodNearestOrigin(HexRodSym, 12, r1, r2, r3);
  OrientationMath::RodtoAxisAngle(r1, r2, r3, w, n1, n2, n3);

  float denom = sqrt((n1*n1+n2*n2+n3*n3));
  n1 = n1/denom;
  n2 = n2/denom;
  n3 = n3/denom;
  if(n3 < 0) n1 = -n1, n2 = -n2, n3 = -n3;
  float newangle = 0;
  float angle = 180.0f*atan2(n2, n1) * DREAM3D::Constants::k_1OverPi;
  if(angle < 0) angle = angle + 360.0f;
  FZn1 = n1;
  FZn2 = n2;
  FZn3 = n3;
  if(angle > 30.0f)
  {
    n1n2mag = sqrt(n1*n1+n2*n2);
    if (int(angle/30)%2 == 0)
    {
      newangle = angle - (30.0f*int(angle/30.0f));
      newangle = newangle* DREAM3D::Constants::k_PiOver180;
      FZn1 = n1n2mag*cosf(newangle);
      FZn2 = n1n2mag*sinf(newangle);
    }
    else
    {
      newangle = angle - (30.0f*int(angle/30.0f));
      newangle = 30.0f - newangle;
      newangle = newangle* DREAM3D::Constants::k_PiOver180;
      FZn1 = n1n2mag*cosf(newangle);
      FZn2 = n1n2mag*sinf(newangle);
    }
  }

  OrientationMath::AxisAngletoRod(w, FZn1, FZn2, FZn3, r1, r2, r3);
}
void HexagonalOps::getNearestQuat(QuatF &q1, QuatF &q2)
{
  int numsym = 12;

  _calcNearestQuat(HexQuatSym, numsym, q1, q2);
}

void HexagonalOps::getFZQuat(QuatF &qr)
{
  int numsym = 12;

  _calcQuatNearestOrigin(HexQuatSym, numsym, qr);
}

int HexagonalOps::getMisoBin(float r1, float r2, float r3)
{
  float dim[3];
  float bins[3];
  float step[3];

  OrientationMath::RodtoHomochoric(r1, r2, r3);

  dim[0] = HexDim1InitValue;
  dim[1] = HexDim2InitValue;
  dim[2] = HexDim3InitValue;
  step[0] = HexDim1StepValue;
  step[1] = HexDim2StepValue;
  step[2] = HexDim3StepValue;
  bins[0] = 36.0f;
  bins[1] = 36.0f;
  bins[2] = 12.0f;

  return _calcMisoBin(dim, bins, step, r1, r2, r3);
}


void HexagonalOps::determineEulerAngles(int choose, float &synea1, float &synea2, float &synea3)
{
  float init[3];
  float step[3];
  float phi[3];
  float r1, r2, r3;

  init[0] = HexDim1InitValue;
  init[1] = HexDim2InitValue;
  init[2] = HexDim3InitValue;
  step[0] = HexDim1StepValue;
  step[1] = HexDim2StepValue;
  step[2] = HexDim3StepValue;
  phi[0] = static_cast<float>(choose % 36);
  phi[1] = static_cast<float>((choose / 36) % 36);
  phi[2] = static_cast<float>(choose / (36 * 36));

  _calcDetermineHomochoricValues(init, step, phi, choose, r1, r2, r3);
  OrientationMath::HomochorictoRod(r1, r2, r3);
  getODFFZRod(r1, r2, r3);
  OrientationMath::RodtoEuler(r1, r2, r3, synea1, synea2, synea3);
}


void HexagonalOps::determineRodriguesVector( int choose, float &r1, float &r2, float &r3)
{
  float init[3];
  float step[3];
  float phi[3];

  init[0] = HexDim1InitValue;
  init[1] = HexDim2InitValue;
  init[2] = HexDim3InitValue;
  step[0] = HexDim1StepValue;
  step[1] = HexDim2StepValue;
  step[2] = HexDim3StepValue;
  phi[0] = static_cast<float>(choose % 36);
  phi[1] = static_cast<float>((choose / 36) % 36);
  phi[2] = static_cast<float>(choose / (36 * 36));

  _calcDetermineHomochoricValues(init, step, phi, choose, r1, r2, r3);
  OrientationMath::HomochorictoRod(r1, r2, r3);
  getMDFFZRod(r1, r2, r3);
}

int HexagonalOps::getOdfBin(float r1, float r2, float r3)
{
  float dim[3];
  float bins[3];
  float step[3];

  OrientationMath::RodtoHomochoric(r1, r2, r3);

  dim[0] = HexDim1InitValue;
  dim[1] = HexDim2InitValue;
  dim[2] = HexDim3InitValue;
  step[0] = HexDim1StepValue;
  step[1] = HexDim2StepValue;
  step[2] = HexDim3StepValue;
  bins[0] = 36.0f;
  bins[1] = 36.0f;
  bins[2] = 12.0f;

  return _calcODFBin(dim, bins, step, r1, r2, r3);
}

void HexagonalOps::getSchmidFactorAndSS(float loadx, float loady, float loadz, float &schmidfactor, int &slipsys)
{
  float theta1, theta2, theta3, theta4, theta5, theta6, theta7, theta8, theta9;
  float lambda1, lambda2, lambda3, lambda4, lambda5, lambda6, lambda7, lambda8, lambda9, lambda10;
  float schmid1, schmid2, schmid3, schmid4, schmid5, schmid6, schmid7, schmid8, schmid9, schmid10, schmid11, schmid12;
  float schmid13, schmid14, schmid15, schmid16, schmid17, schmid18, schmid19, schmid20, schmid21, schmid22, schmid23, schmid24;
  float caratio = 1.633f;
  float ph1sdx1 = 1.0f;
  float ph1sdy1 = 0.0f;
  float ph1sdz1 = 0.0f;
  float ph1sdx2 = 0.0f;
  float ph1sdy2 = 1.0f;
  float ph1sdz2 = 0.0f;
  float ph1sdx3 = -0.707f;
  float ph1sdy3 = -0.707f;
  float ph1sdz3 = 0.0f;
  float ph1sdx4 = 0.0f;
  float ph1sdy4 = -0.707f;
  float ph1sdz4 = 0.707f;
  float ph1sdx5 = -0.57735f;
  float ph1sdy5 = -0.57735f;
  float ph1sdz5 = 0.57735f;
  float ph1sdx6 = 0.707f;
  float ph1sdy6 = 0.0f;
  float ph1sdz6 = 0.707f;
  float ph1sdx7 = 0.57735f;
  float ph1sdy7 = 0.57735f;
  float ph1sdz7 = 0.57735f;
  float ph1sdx8 = 0.0f;
  float ph1sdy8 = 0.707f;
  float ph1sdz8 = 0.707f;
  float ph1sdx9 = -0.707f;
  float ph1sdy9 = 0.0f;
  float ph1sdz9 = 0.707f;
  float ph1spnx1 = 0.0f;
  float ph1spny1 = 0.0f;
  float ph1spnz1 = 1.0f;
  float ph1spnx2 = 0.4472f;
  float ph1spny2 = 0.8944f;
  float ph1spnz2 = 0.0f;
  float ph1spnx3 = 0.8944f;
  float ph1spny3 = 0.4472f;
  float ph1spnz3 = 0.0f;
  float ph1spnx4 = -0.707f;
  float ph1spny4 = 0.707f;
  float ph1spnz4 = 0.0f;
  float ph1spnx5 = 0.4082f;
  float ph1spny5 = 0.8164f;
  //	float ph1spnz5 = -0.4082f;
  float ph1spnx6 = 0.4082f;
  float ph1spny6 = 0.8164f;
  //	float ph1spnz6 = 0.4082f;
  float ph1spnx7 = 0.8164f;
  float ph1spny7 = 0.4082f;
  //	float ph1spnz7 = -0.4082f;
  float ph1spnx8 = 0.8164f;
  float ph1spny8 = 0.4082f;
  //	float ph1spnz8 = 0.4082f;
  float ph1spnx9 = -0.57735f;
  float ph1spny9 = 0.57735f;
  //	float ph1spnz9 = -0.57735f;
  float ph1spnx10 = -0.57735f;
  float ph1spny10 = 0.57735f;
  //	float ph1spnz10 = 0.57735f;
  float t1x = (0.866025f*ph1sdx1)+(0.0f*ph1sdy1)+(0.0f*ph1sdz1);
  float t1y = (-0.5f*ph1sdx1)+(1.0f*ph1sdy1)+(0.0f*ph1sdz1);
  float t1z = (0.0f*ph1sdx1)+(0.0f*ph1sdy1)+(caratio*ph1sdz1);
  float denomt1 = powf((t1x*t1x+t1y*t1y+t1z*t1z),0.5f);
  t1x = t1x/denomt1;
  t1y = t1y/denomt1;
  t1z = t1z/denomt1;
  theta1 = ((t1x*loadx)+(t1y*loady)+(t1z*loadz));
  theta1 = fabs(theta1);
  float t2x = (0.866025f*ph1sdx2)+(0.0f*ph1sdy2)+(0.0f*ph1sdz2);
  float t2y = (-0.5f*ph1sdx2)+(1.0f*ph1sdy2)+(0.0f*ph1sdz2);
  float t2z = (0.0f*ph1sdx2)+(0.0f*ph1sdy2)+(caratio*ph1sdz2);
  float denomt2 = powf((t2x*t2x+t2y*t2y+t2z*t2z),0.5f);
  t2x = t2x/denomt2;
  t2y = t2y/denomt2;
  t2z = t2z/denomt2;
  theta2 = ((t2x*loadx)+(t2y*loady)+(t2z*loadz));
  theta2 = fabs(theta2);
  float t3x = (0.866025f*ph1sdx3)+(0.0f*ph1sdy3)+(0.0f*ph1sdz3);
  float t3y = (-0.5f*ph1sdx3)+(1.0f*ph1sdy3)+(0.0f*ph1sdz3);
  float t3z = (0.0f*ph1sdx3)+(0.0f*ph1sdy3)+(caratio*ph1sdz3);
  float denomt3 = powf((t3x*t3x+t3y*t3y+t3z*t3z),0.5f);
  t3x = t3x/denomt3;
  t3y = t3y/denomt3;
  t3z = t3z/denomt3;
  theta3 = ((t3x*loadx)+(t3y*loady)+(t3z*loadz));
  theta3 = fabs(theta3);
  float l1nx = (0.866025f*ph1spnx1)+(0.0f*ph1spny1);
  float l1ny = (-0.5f*ph1spnx1)+(1.0f*ph1spny1);
  float l1nz = -caratio*ph1spnz1;
  float denoml1 = powf((l1nx*l1nx+l1ny*l1ny+l1nz*l1nz),0.5f);
  l1nx = l1nx/denoml1;
  l1ny = l1ny/denoml1;
  l1nz = l1nz/denoml1;
  lambda1 = ((l1nx*loadx)+(l1ny*loady)+(l1nz*loadz));
  lambda1 = fabs(lambda1);
  schmid1 = theta1*lambda1;
  schmid2 = theta2*lambda1;
  schmid3 = theta3*lambda1;
  float l2nx = (0.866025f*ph1spnx2)+(0.0f*ph1spny2);
  float l2ny = (-0.5f*ph1spnx2)+(1*ph1spny2);
  float l2nz = -caratio*ph1spnz2;
  float denoml2 = powf((l2nx*l2nx+l2ny*l2ny+l2nz*l2nz),0.5f);
  l2nx = l2nx/denoml2;
  l2ny = l2ny/denoml2;
  l2nz = l2nz/denoml2;
  lambda2 = ((l2nx*loadx)+(l2ny*loady)+(l2nz*loadz));
  lambda2 = fabs(lambda2);
  float l3nx = (0.866025f*ph1spnx3)+(0.0f*ph1spny3);
  float l3ny = (-0.5f*ph1spnx3)+(1.0f*ph1spny3);
  float l3nz = -caratio*ph1spnz3;
  float denoml3 = powf((l3nx*l3nx+l3ny*l3ny+l3nz*l3nz),0.5f);
  l3nx = l3nx/denoml3;
  l3ny = l3ny/denoml3;
  l3nz = l3nz/denoml3;
  lambda3 = ((l3nx*loadx)+(l3ny*loady)+(l3nz*loadz));
  lambda3 = fabs(lambda3);
  float l4nx = (0.866025f*ph1spnx4)+(0.0f*ph1spny4);
  float l4ny = (-0.5f*ph1spnx4)+(1*ph1spny4);
  float l4nz = -caratio*ph1spnz4;
  float denoml4 = powf((l4nx*l4nx+l4ny*l4ny+l4nz*l4nz),0.5f);
  l4nx = l4nx/denoml4;
  l4ny = l4ny/denoml4;
  l4nz = l4nz/denoml4;
  lambda4 = ((l4nx*loadx)+(l4ny*loady)+(l4nz*loadz));
  lambda4 = fabs(lambda4);
  schmid4 = theta1*lambda2;
  schmid5 = theta2*lambda3;
  schmid6 = theta3*lambda4;
  float l5nx = (0.866025f*ph1spnx5)+(0.0f*ph1spny5);
  float l5ny = (-0.5f*ph1spnx5)+(1*ph1spny5);
  float l5nz = float((l5nx*-l5nx+l5ny*-l5ny))/(caratio*0.8164f);
  float denoml5 = powf((l5nx*l5nx+l5ny*l5ny+l5nz*l5nz),0.5f);
  l5nx = l5nx/denoml5;
  l5ny = l5ny/denoml5;
  l5nz = l5nz/denoml5;
  float l6nx = (0.866025f*ph1spnx6)+(0.0f*ph1spny6);
  float l6ny = (-0.5f*ph1spnx6)+(1.0f*ph1spny6);
  float l6nz = float(-(l6nx*-l6nx+l6ny*-l6ny))/(caratio*0.8164f);
  float denoml6 = powf((l6nx*l6nx+l6ny*l6ny+l6nz*l6nz),0.5f);
  l6nx = l6nx/denoml6;
  l6ny = l6ny/denoml6;
  l6nz = l6nz/denoml6;
  float l7nx = (0.866025f*ph1spnx7)+(0.0f*ph1spny7);
  float l7ny = (-0.5f*ph1spnx7)+(1.0f*ph1spny7);
  float l7nz = float((l7nx*-l7nx+l7ny*-l7ny))/(caratio*0.8164f);
  float denoml7 = powf((l7nx*l7nx+l7ny*l7ny+l7nz*l7nz),0.5f);
  l7nx = l7nx/denoml7;
  l7ny = l7ny/denoml7;
  l7nz = l7nz/denoml7;
  float l8nx = (0.866025f*ph1spnx8)+(0*ph1spny8);
  float l8ny = (-0.5f*ph1spnx8)+(1.0f*ph1spny8);
  float l8nz = float(-(l8nx*-l8nx+l8ny*-l8ny))/(caratio*0.8164f);
  float denoml8 = powf((l8nx*l8nx+l8ny*l8ny+l8nz*l8nz),0.5f);
  l8nx = l8nx/denoml8;
  l8ny = l8ny/denoml8;
  l8nz = l8nz/denoml8;
  float l9nx = (0.866025f*ph1spnx9)+(0.0f*ph1spny9);
  float l9ny = (-0.5f*ph1spnx9)+(1.0f*ph1spny9);
  float l9nz = float((l9nx*-l9nx+l9ny*-l9ny))/(caratio*1.154f);
  float denoml9 = powf((l9nx*l9nx+l9ny*l9ny+l9nz*l9nz),0.5f);
  l9nx = l9nx/denoml9;
  l9ny = l9ny/denoml9;
  l9nz = l9nz/denoml9;
  float l10nx = (0.866025f*ph1spnx10)+(0.0f*ph1spny10);
  float l10ny = (-0.5f*ph1spnx10)+(1*ph1spny10);
  float l10nz = float(-(l10nx*-l10nx+l10ny*-l10ny))/(caratio*1.154f);
  float denoml10 = powf((l10nx*l10nx+l10ny*l10ny+l10nz*l10nz),0.5f);
  l10nx = l10nx/denoml10;
  l10ny = l10ny/denoml10;
  l10nz = l10nz/denoml10;
  lambda5 = ((l5nx*loadx)+(l5ny*loady)+(-l5nz*loadz));
  lambda5 = fabs(lambda5);
  lambda6 = ((l5nx*loadx)+(l5ny*loady)+(l5nz*loadz));
  lambda6 = fabs(lambda6);
  lambda7 = ((l7nx*loadx)+(l7ny*loady)+(-l7nz*loadz));
  lambda7 = fabs(lambda7);
  lambda8 = ((l7nx*loadx)+(l7ny*loady)+(l7nz*loadz));
  lambda8 = fabs(lambda8);
  lambda9 = ((l9nx*loadx)+(l9ny*loady)+(-l9nz*loadz));
  lambda9 = fabs(lambda9);
  lambda10 = ((l9nx*loadx)+(l9ny*loady)+(l9nz*loadz));
  lambda10 = fabs(lambda10);
  schmid7 = theta1*lambda5;
  schmid8 = theta1*lambda6;
  schmid9 = theta2*lambda7;
  schmid10 = theta2*lambda8;
  schmid11 = theta3*lambda9;
  schmid12 = theta3*lambda10;
  float t4x = (0.866025f*ph1sdx4)+(0.0f*ph1sdy4)+(0.0f*ph1sdz4);
  float t4y = (-0.5f*ph1sdx4)+(1.0f*ph1sdy4)+(0.0f*ph1sdz4);
  float t4z = (0.0f*ph1sdx4)+(0.0f*ph1sdy4)+(caratio*ph1sdz4);
  float denomt4 = powf((t4x*t4x+t4y*t4y+t4z*t4z),0.5f);
  t4x = t4x/denomt4;
  t4y = t4y/denomt4;
  t4z = t4z/denomt4;
  theta4 = ((t4x*loadx)+(t4y*loady)+(t4z*loadz));
  theta4 = fabs(theta4);
  float t5x = (0.866025f*ph1sdx5)+(0.0f*ph1sdy5)+(0.0f*ph1sdz5);
  float t5y = (-0.5f*ph1sdx5)+(1.0f*ph1sdy5)+(0.0f*ph1sdz5);
  float t5z = (0.0f*ph1sdx5)+(0.0f*ph1sdy5)+(caratio*ph1sdz5);
  float denomt5 = powf((t5x*t5x+t5y*t5y+t5z*t5z),0.5f);
  t5x = t5x/denomt5;
  t5y = t5y/denomt5;
  t5z = t5z/denomt5;
  theta5 = ((t5x*loadx)+(t5y*loady)+(t5z*loadz));
  theta5 = fabs(theta5);
  float t6x = (0.866025f*ph1sdx6)+(0.0f*ph1sdy6)+(0.0f*ph1sdz6);
  float t6y = (-0.5f*ph1sdx6)+(1.0f*ph1sdy6)+(0.0f*ph1sdz6);
  float t6z = (0.0f*ph1sdx6)+(0.0f*ph1sdy6)+(caratio*ph1sdz6);
  float denomt6 = powf((t6x*t6x+t6y*t6y+t6z*t6z),0.5f);
  t6x = t6x/denomt6;
  t6y = t6y/denomt6;
  t6z = t6z/denomt6;
  theta6 = ((t6x*loadx)+(t6y*loady)+(t6z*loadz));
  theta6 = fabs(theta6);
  float t7x = (0.866025f*ph1sdx7)+(0.0f*ph1sdy7)+(0.0f*ph1sdz7);
  float t7y = (-0.5f*ph1sdx7)+(1.0f*ph1sdy7)+(0.0f*ph1sdz7);
  float t7z = (0.0f*ph1sdx7)+(0.0f*ph1sdy7)+(caratio*ph1sdz7);
  float denomt7 = powf((t7x*t7x+t7y*t7y+t7z*t7z),0.5f);
  t7x = t7x/denomt7;
  t7y = t7y/denomt7;
  t7z = t7z/denomt7;
  theta7 = ((t7x*loadx)+(t7y*loady)+(t7z*loadz));
  theta7 = fabs(theta7);
  float t8x = (0.866025f*ph1sdx8)+(0.0f*ph1sdy8)+(0.0f*ph1sdz8);
  float t8y = (-0.5f*ph1sdx8)+(1.0f*ph1sdy8)+(0.0f*ph1sdz8);
  float t8z = (0.0f*ph1sdx8)+(0.0f*ph1sdy8)+(caratio*ph1sdz8);
  float denomt8 = powf((t8x*t8x+t8y*t8y+t8z*t8z),0.5f);
  t8x = t8x/denomt8;
  t8y = t8y/denomt8;
  t8z = t8z/denomt8;
  theta8 = ((t8x*loadx)+(t8y*loady)+(t8z*loadz));
  theta8 = fabs(theta8);
  float t9x = (0.866025f*ph1sdx9)+(0.0f*ph1sdy9)+(0.0f*ph1sdz9);
  float t9y = (-0.5f*ph1sdx9)+(1*ph1sdy9)+(0*ph1sdz9);
  float t9z = (0.0f*ph1sdx9)+(0.0f*ph1sdy9)+(caratio*ph1sdz9);
  float denomt9 = powf((t9x*t9x+t9y*t9y+t9z*t9z),0.5f);
  t9x = t9x/denomt9;
  t9y = t9y/denomt9;
  t9z = t9z/denomt9;
  theta9 = ((t9x*loadx)+(t9y*loady)+(t9z*loadz));
  theta9 = fabs(theta9);
  schmid13 = theta7*lambda6;
  schmid14 = theta8*lambda6;
  schmid15 = theta6*lambda9;
  schmid16 = theta4*lambda9;
  schmid17 = theta6*lambda8;
  schmid18 = theta7*lambda8;
  schmid19 = theta4*lambda5;
  schmid20 = theta5*lambda5;
  schmid21 = theta8*lambda10;
  schmid22 = theta9*lambda10;
  schmid23 = theta9*lambda7;
  schmid24 = theta5*lambda7;
  if(schmid1 > schmidfactor) schmidfactor = schmid1, slipsys = 1;
  if(schmid2 > schmidfactor) schmidfactor = schmid2, slipsys = 2;
  if(schmid3 > schmidfactor) schmidfactor = schmid3, slipsys = 3;
  if(schmid4 > schmidfactor) schmidfactor = schmid4, slipsys = 4;
  if(schmid5 > schmidfactor) schmidfactor = schmid5, slipsys = 5;
  if(schmid6 > schmidfactor) schmidfactor = schmid6, slipsys = 6;
  if(schmid7 > schmidfactor) schmidfactor = schmid7, slipsys = 7;
  if(schmid8 > schmidfactor) schmidfactor = schmid8, slipsys = 8;
  if(schmid9 > schmidfactor) schmidfactor = schmid9, slipsys = 9;
  if(schmid10 > schmidfactor) schmidfactor = schmid10, slipsys = 10;
  if(schmid11 > schmidfactor) schmidfactor = schmid11, slipsys = 11;
  if(schmid12 > schmidfactor) schmidfactor = schmid12, slipsys = 12;
  if(schmid13 > schmidfactor) schmidfactor = schmid13, slipsys = 13;
  if(schmid14 > schmidfactor) schmidfactor = schmid14, slipsys = 14;
  if(schmid15 > schmidfactor) schmidfactor = schmid15, slipsys = 15;
  if(schmid16 > schmidfactor) schmidfactor = schmid16, slipsys = 16;
  if(schmid17 > schmidfactor) schmidfactor = schmid17, slipsys = 17;
  if(schmid18 > schmidfactor) schmidfactor = schmid18, slipsys = 18;
  if(schmid19 > schmidfactor) schmidfactor = schmid19, slipsys = 19;
  if(schmid20 > schmidfactor) schmidfactor = schmid20, slipsys = 20;
  if(schmid21 > schmidfactor) schmidfactor = schmid21, slipsys = 21;
  if(schmid22 > schmidfactor) schmidfactor = schmid22, slipsys = 22;
  if(schmid23 > schmidfactor) schmidfactor = schmid23, slipsys = 23;
  if(schmid24 > schmidfactor) schmidfactor = schmid24, slipsys = 24;
}

void HexagonalOps::getmPrime(QuatF &q1, QuatF &q2, float LD[3], float &mPrime)
{
  BOOST_ASSERT(false);
#if 0
  /* I am asserting here because this code will simply give junk results and if someone uses it
   * they could unknowningly get really bad results
   */
  float g1[3][3];
  float g2[3][3];
  float h1, k1, l1, u1, v1, w1;
  float h2, k2, l2, u2, v2, w2;
  float denomhkl1, denomhkl2, denomuvw1, denomuvw2;
  float planemisalignment, directionmisalignment;
  QuattoMat(q1, g1);
  QuattoMat(q2, g2);
  // Note the order of multiplication is such that I am actually multiplying by the inverse of g1 and g2
  /*  h1 = CubicSlipSystems[ss1][0]*g1[0][0]+CubicSlipSystems[ss1][1]*g1[1][0]+CubicSlipSystems[ss1][2]*g1[2][0];
  k1 = CubicSlipSystems[ss1][0]*g1[0][1]+CubicSlipSystems[ss1][1]*g1[1][1]+CubicSlipSystems[ss1][2]*g1[2][1];
  l1 = CubicSlipSystems[ss1][0]*g1[0][2]+CubicSlipSystems[ss1][1]*g1[1][2]+CubicSlipSystems[ss1][2]*g1[2][2];
  u1 = CubicSlipSystems[ss1][3]*g1[0][0]+CubicSlipSystems[ss1][4]*g1[1][0]+CubicSlipSystems[ss1][5]*g1[2][0];
  v1 = CubicSlipSystems[ss1][3]*g1[0][1]+CubicSlipSystems[ss1][4]*g1[1][1]+CubicSlipSystems[ss1][5]*g1[2][1];
  w1 = CubicSlipSystems[ss1][3]*g1[0][2]+CubicSlipSystems[ss1][4]*g1[1][2]+CubicSlipSystems[ss1][5]*g1[2][2];
  denomhkl1 = sqrtf((h1*h1+k1*k1+l1*l1));
  denomuvw1 = sqrtf((u1*u1+v1*v1+w1*w1));
  h2 = CubicSlipSystems[ss2][0]*g2[0][0]+CubicSlipSystems[ss2][1]*g2[1][0]+CubicSlipSystems[ss2][2]*g2[2][0];
  k2 = CubicSlipSystems[ss2][0]*g2[0][1]+CubicSlipSystems[ss2][1]*g2[1][1]+CubicSlipSystems[ss2][2]*g2[2][1];
  l2 = CubicSlipSystems[ss2][0]*g2[0][2]+CubicSlipSystems[ss2][1]*g2[1][2]+CubicSlipSystems[ss2][2]*g2[2][2];
  u2 = CubicSlipSystems[ss2][3]*g2[0][0]+CubicSlipSystems[ss2][4]*g2[1][0]+CubicSlipSystems[ss2][5]*g2[2][0];
  v2 = CubicSlipSystems[ss2][3]*g2[0][1]+CubicSlipSystems[ss2][4]*g2[1][1]+CubicSlipSystems[ss2][5]*g2[2][1];
  w2 = CubicSlipSystems[ss2][3]*g2[0][2]+CubicSlipSystems[ss2][4]*g2[1][2]+CubicSlipSystems[ss2][5]*g2[2][2];
*/
  denomhkl2 = sqrtf((h2*h2+k2*k2+l2*l2));
  denomuvw2 = sqrtf((u2*u2+v2*v2+w2*w2));
  planemisalignment = fabs((h1*h2+k1*k2+l1*l2)/(denomhkl1*denomhkl2));
  directionmisalignment = fabs((u1*u2+v1*v2+w1*w2)/(denomuvw1*denomuvw2));
  mPrime = planemisalignment*directionmisalignment;
#endif
}

void HexagonalOps::getF1(QuatF &q1, QuatF &q2, float LD[3], bool maxSF, float &F1)
{
  BOOST_ASSERT(false);
#if 0
  /* I am asserting here because this code will simply give junk results and if someone uses it
   * they could unknowningly get really bad results
   */
  float g1[3][3];
  float g2[3][3];
  //  float hkl1[3], uvw1[3];
  //  float hkl2[3], uvw2[3];
  //  float slipDirection[3], slipPlane[3];
  //  float denomhkl1=0, denomhkl2=0, denomuvw1=0, denomuvw2=0;
  //  float directionMisalignment=0, totalDirectionMisalignment=0;
  //  float schmidFactor1=0, schmidFactor2=0;
  float maxSchmidFactor=0;
  //  float directionComponent1=0, planeComponent1=0;
  //  float directionComponent2=0, planeComponent2=0;
  //  float maxF1=0;

  QuattoMat(q1, g1);
  QuattoMat(q2, g2);
  MatrixMath::Normalize3x1(LD);
  // Note the order of multiplication is such that I am actually multiplying by the inverse of g1 and g2
  if(maxSF == true) maxSchmidFactor = 0;
  /*  for(int i=0;i<12;i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1,slipDirection,hkl1);
    MatrixMath::Multiply3x3with3x1(g1,slipPlane,uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = MatrixMath::DotProduct(LD,uvw1);
    planeComponent1 = MatrixMath::DotProduct(LD,hkl1);
    schmidFactor1 = directionComponent1*planeComponent1;
    if(schmidFactor1 > maxSchmidFactor || maxSF == false)
    {
      totalDirectionMisalignment = 0;
      if(maxSF == true) maxSchmidFactor = schmidFactor1;
      for(int j=0;j<12;j++)
      {
        slipDirection[0] = CubicSlipDirections[i][0];
        slipDirection[1] = CubicSlipDirections[i][1];
        slipDirection[2] = CubicSlipDirections[i][2];
        slipPlane[0] = CubicSlipPlanes[i][0];
        slipPlane[1] = CubicSlipPlanes[i][1];
        slipPlane[2] = CubicSlipPlanes[i][2];
        MatrixMath::Multiply3x3with3x1(g2,slipDirection,hkl2);
        MatrixMath::Multiply3x3with3x1(g2,slipPlane,uvw2);
        MatrixMath::Normalize3x1(hkl2);
        MatrixMath::Normalize3x1(uvw2);
        directionComponent2 = MatrixMath::DotProduct(LD,uvw2);
        planeComponent2 = MatrixMath::DotProduct(LD,hkl2);
        schmidFactor2 = directionComponent2*planeComponent2;
        totalDirectionMisalignment = totalDirectionMisalignment + directionMisalignment;
      }
      F1 = schmidFactor1*directionComponent1*totalDirectionMisalignment;
      if(maxSF == false)
      {
        if(F1 < maxF1) F1 = maxF1;
        else maxF1 = F1;
      }
    }
  }
*/
#endif
}
void HexagonalOps::getF1spt(QuatF &q1, QuatF &q2, float LD[3], bool maxSF, float &F1spt)
{
  BOOST_ASSERT(false);
#if 0
  float g1[3][3];
  float g2[3][3];
  //  float hkl1[3], uvw1[3];
  //  float hkl2[3], uvw2[3];
  //  float slipDirection[3], slipPlane[3];
  //  float directionMisalignment=0, totalDirectionMisalignment=0;
  //  float planeMisalignment=0, totalPlaneMisalignment=0;
  //  float schmidFactor1=0, schmidFactor2=0;
  float maxSchmidFactor=0;
  //  float directionComponent1=0, planeComponent1=0;
  //  float directionComponent2=0, planeComponent2=0;
  //  float maxF1spt=0;

  QuattoMat(q1, g1);
  QuattoMat(q2, g2);
  MatrixMath::Normalize3x1(LD);
  // Note the order of multiplication is such that I am actually multiplying by the inverse of g1 and g2
  if(maxSF == true) maxSchmidFactor = 0;
  /*  for(int i=0;i<12;i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1,slipDirection,hkl1);
    MatrixMath::Multiply3x3with3x1(g1,slipPlane,uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = MatrixMath::DotProduct(LD,uvw1);
    planeComponent1 = MatrixMath::DotProduct(LD,hkl1);
    schmidFactor1 = directionComponent1*planeComponent1;
    if(schmidFactor1 > maxSchmidFactor || maxSF == false)
    {
      totalDirectionMisalignment = 0;
      totalPlaneMisalignment = 0;
      if(maxSF == true) maxSchmidFactor = schmidFactor1;
      for(int j=0;j<12;j++)
      {
        slipDirection[0] = CubicSlipDirections[j][0];
        slipDirection[1] = CubicSlipDirections[j][1];
        slipDirection[2] = CubicSlipDirections[j][2];
        slipPlane[0] = CubicSlipPlanes[j][0];
        slipPlane[1] = CubicSlipPlanes[j][1];
        slipPlane[2] = CubicSlipPlanes[j][2];
        MatrixMath::Multiply3x3with3x1(g2,slipDirection,hkl2);
        MatrixMath::Multiply3x3with3x1(g2,slipPlane,uvw2);
        MatrixMath::Normalize3x1(hkl2);
        MatrixMath::Normalize3x1(uvw2);
        directionComponent2 = MatrixMath::DotProduct(LD,uvw2);
        planeComponent2 = MatrixMath::DotProduct(LD,hkl2);
        schmidFactor2 = directionComponent2*planeComponent2;
        directionMisalignment = fabs(MatrixMath::DotProduct(uvw1,uvw2));
        planeMisalignment = fabs(MatrixMath::DotProduct(hkl1,hkl2));
        totalDirectionMisalignment = totalDirectionMisalignment + directionMisalignment;
        totalPlaneMisalignment = totalPlaneMisalignment + planeMisalignment;
      }
      F1spt = schmidFactor1*directionComponent1*totalDirectionMisalignment*totalPlaneMisalignment;
      if(maxSF == false)
      {
        if(F1spt < maxF1spt) F1spt = maxF1spt;
        else maxF1spt = F1spt;
      }
    }
  }
*/
#endif
}

void HexagonalOps::getF7(QuatF &q1, QuatF &q2, float LD[3], bool maxSF, float &F7)
{
  BOOST_ASSERT(false);
#if 0
  float g1[3][3];
  float g2[3][3];
  //  float hkl1[3], uvw1[3];
  //  float hkl2[3], uvw2[3];
  //  float slipDirection[3], slipPlane[3];
  //  float directionMisalignment=0, totalDirectionMisalignment=0;
  //  float schmidFactor1=0, schmidFactor2=0, maxSchmidFactor=0;
  //  float directionComponent1=0, planeComponent1=0;
  //  float directionComponent2=0, planeComponent2=0;
  //  float maxF7=0;

  QuattoMat(q1, g1);
  QuattoMat(q2, g2);
  MatrixMath::Normalize3x1(LD);
  // Note the order of multiplication is such that I am actually multiplying by the inverse of g1 and g2

  /*  for(int i=0;i<12;i++)
  {
    slipDirection[0] = CubicSlipDirections[i][0];
    slipDirection[1] = CubicSlipDirections[i][1];
    slipDirection[2] = CubicSlipDirections[i][2];
    slipPlane[0] = CubicSlipPlanes[i][0];
    slipPlane[1] = CubicSlipPlanes[i][1];
    slipPlane[2] = CubicSlipPlanes[i][2];
    MatrixMath::Multiply3x3with3x1(g1,slipDirection,hkl1);
    MatrixMath::Multiply3x3with3x1(g1,slipPlane,uvw1);
    MatrixMath::Normalize3x1(hkl1);
    MatrixMath::Normalize3x1(uvw1);
    directionComponent1 = MatrixMath::DotProduct(LD,uvw1);
    planeComponent1 = MatrixMath::DotProduct(LD,hkl1);
    schmidFactor1 = directionComponent1*planeComponent1;
    if(schmidFactor1 > maxSchmidFactor || maxSF == false)
    {
      totalDirectionMisalignment = 0;
      if(maxSF == true) maxSchmidFactor = schmidFactor1;
      for(int j=0;j<12;j++)
      {
        slipDirection[0] = CubicSlipDirections[j][0];
        slipDirection[1] = CubicSlipDirections[j][1];
        slipDirection[2] = CubicSlipDirections[j][2];
        slipPlane[0] = CubicSlipPlanes[j][0];
        slipPlane[1] = CubicSlipPlanes[j][1];
        slipPlane[2] = CubicSlipPlanes[j][2];
        MatrixMath::Multiply3x3with3x1(g2,slipDirection,hkl2);
        MatrixMath::Multiply3x3with3x1(g2,slipPlane,uvw2);
        MatrixMath::Normalize3x1(hkl2);
        MatrixMath::Normalize3x1(uvw2);
        directionComponent2 = MatrixMath::DotProduct(LD,uvw2);
        planeComponent2 = MatrixMath::DotProduct(LD,hkl2);
        schmidFactor2 = directionComponent2*planeComponent2;
        totalDirectionMisalignment = totalDirectionMisalignment + directionMisalignment;
      }
      F7 = directionComponent1*directionComponent1*totalDirectionMisalignment;
      if(maxSF == false)
      {
        if(F7 < maxF7) F7 = maxF7;
        else maxF7 = F7;
      }
    }
  }
*/
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void HexagonalOps::generateSphereCoordsFromEulers(FloatArrayType *eulers, FloatArrayType *xyz0001, FloatArrayType *xyz1010, FloatArrayType *xyz1120)
{
  size_t nOrientations = eulers->GetNumberOfTuples();
  HexagonalOps ops;
  float g[3][3];
  float gTranpose[3][3];
  float* currentEuler = NULL;
  float direction[3] = {0.0, 0.0, 0.0};

  // Sanity Check the size of the arrays
  if (xyz0001->GetNumberOfTuples() < nOrientations * 2)
  {
    xyz0001->Resize(nOrientations * 2 * 3);
  }
  if (xyz1010->GetNumberOfTuples() < nOrientations * 6)
  {
    xyz1010->Resize(nOrientations * 6 * 3);
  }
  if (xyz1120->GetNumberOfTuples() < nOrientations * 6)
  {
    xyz1120->Resize(nOrientations * 6 * 3);
  }

  // Geneate all the Coordinates
  for(size_t i = 0; i < nOrientations; ++i)
  {

    currentEuler = eulers->GetPointer(i * 3);

    OrientationMath::EulertoMat(currentEuler[0], currentEuler[1], currentEuler[2], g);
    MatrixMath::Transpose3x3(g, gTranpose);

    // -----------------------------------------------------------------------------
    // 0001 Family
    direction[0] = 0.0; direction[1] = 0.0; direction[2] = 1.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz0001->GetPointer(i*6));
    MatrixMath::Copy3x1(xyz0001->GetPointer(i*6),xyz0001->GetPointer(i*6 + 3));
    MatrixMath::Multiply3x1withConstant(xyz0001->GetPointer(i*6 + 3),-1);

    // -----------------------------------------------------------------------------
    // 1010 Family
    direction[0] = DREAM3D::Constants::k_Root3Over2; direction[1] = 0.5; direction[2] = 0.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz1010->GetPointer(i*18));
    MatrixMath::Copy3x1(xyz1010->GetPointer(i*18),xyz1010->GetPointer(i*18 + 3));
    MatrixMath::Multiply3x1withConstant(xyz1010->GetPointer(i*18 + 3),-1);
    direction[0] = 0.0; direction[1] = 1.0; direction[2] = 0.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz1010->GetPointer(i*18 + 6));
    MatrixMath::Copy3x1(xyz1010->GetPointer(i*18+6),xyz1010->GetPointer(i*18 + 9));
    MatrixMath::Multiply3x1withConstant(xyz1010->GetPointer(i*18 + 9),-1);
    direction[0] = -DREAM3D::Constants::k_Root3Over2; direction[1] = 0.5; direction[2] = 0.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz1010->GetPointer(i*18 + 12));
    MatrixMath::Copy3x1(xyz1010->GetPointer(i*18+12),xyz1010->GetPointer(i*18 + 15));
    MatrixMath::Multiply3x1withConstant(xyz1010->GetPointer(i*18 + 15),-1);

    // -----------------------------------------------------------------------------
    // 1120 Family
    direction[0] = 1.0; direction[1] = 0.0; direction[2] = 0.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz1120->GetPointer(i*18));
    MatrixMath::Copy3x1(xyz1120->GetPointer(i*18),xyz1120->GetPointer(i*18 + 3));
    MatrixMath::Multiply3x1withConstant(xyz1120->GetPointer(i*18 + 3),-1);
    direction[0] = 0.5; direction[1] = DREAM3D::Constants::k_Root3Over2; direction[2] = 0.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz1120->GetPointer(i*18 + 6));
    MatrixMath::Copy3x1(xyz1120->GetPointer(i*18+6),xyz1120->GetPointer(i*18 + 9));
    MatrixMath::Multiply3x1withConstant(xyz1120->GetPointer(i*18 + 9),-1);
    direction[0] = -0.5; direction[1] = DREAM3D::Constants::k_Root3Over2; direction[2] = 0.0;
    MatrixMath::Multiply3x3with3x1(gTranpose, direction, xyz1120->GetPointer(i*18 + 12));
    MatrixMath::Copy3x1(xyz1120->GetPointer(i*18+12),xyz1120->GetPointer(i*18 + 15));
    MatrixMath::Multiply3x1withConstant(xyz1120->GetPointer(i*18 + 15),-1);
  }

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void HexagonalOps::generateIPFColor(double* eulers, double* refDir, uint8_t* rgb, bool convertDegrees)
{
  generateIPFColor(eulers[0], eulers[1], eulers[2], refDir[0], refDir[1], refDir[2], rgb, convertDegrees);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void HexagonalOps::generateIPFColor(double phi1, double phi, double phi2, double refDir0, double refDir1, double refDir2, uint8_t* rgb, bool degToRad)
{
  if (degToRad == true)
  {
    phi1 = phi1 * DREAM3D::Constants::k_DegToRad;
    phi = phi * DREAM3D::Constants::k_DegToRad;
    phi2 = phi2 * DREAM3D::Constants::k_DegToRad;
  }

  QuatF qc;
  QuatF q1;
  float g[3][3];
  float p[3];
  float refDirection[3];
  float d[3];
  float theta, phi_local;
  float _rgb[3] = { 0.0, 0.0, 0.0 };

  OrientationMath::EulertoQuat(q1, phi1, phi, phi2);

  for (int j = 0; j < 12; j++)
  {
    QuaternionMathF::Multiply(q1, HexQuatSym[j], qc);

    OrientationMath::QuattoMat(qc, g);

    refDirection[0] = refDir0;
    refDirection[1] = refDir1;
    refDirection[2] = refDir2;
    MatrixMath::Multiply3x3with3x1(g, refDirection, p);
    MatrixMath::Normalize3x1(p);

    if (p[2] < 0)
    {
      p[0] = -p[0];
      p[1] = -p[1];
      p[2] = -p[2];
    }
    d[0] = p[0];
    d[1] = p[1];
    d[2] = 0;
    MatrixMath::Normalize3x1(d);
    if (atan2(d[1], d[0]) >= 0 && atan2(d[1], d[0]) < (30.0 * DREAM3D::Constants::k_DegToRad))
    {
      theta = (p[0] * 0) + (p[1] * 0) + (p[2] * 1);
      if (theta > 1) theta = 1;

      if (theta < -1) theta = -1;

      theta = (DREAM3D::Constants::k_RadToDeg) * acos(theta);
      _rgb[0] = (90.0f - theta) / 90.0f;
      phi_local = (d[0] * 1) + (d[1] * 0) + (d[2] * 0);
      if (phi_local > 1) phi_local = 1;

      if (phi_local < -1) phi_local = -1;

      phi_local = (DREAM3D::Constants::k_RadToDeg) * acos(phi_local);
      _rgb[1] = (1 - _rgb[0]) * ((30.0f - phi_local) / 30.0f);
      _rgb[2] = (1 - _rgb[0]) - _rgb[1];
      break;
    }
  }

  float max = _rgb[0];
  if (_rgb[1] > max) max = _rgb[1];
  if (_rgb[2] > max) max = _rgb[2];

  _rgb[0] = _rgb[0] / max;
  _rgb[1] = _rgb[1] / max;
  _rgb[2] = _rgb[2] / max;
//  _rgb[0] = (0.85f * _rgb[0]) + 0.15f;
//  _rgb[1] = (0.85f * _rgb[1]) + 0.15f;
//  _rgb[2] = (0.85f * _rgb[2]) + 0.15f;

  // Multiply by 255 to get an R/G/B value
  _rgb[0] = _rgb[0] * 255.0f;
  _rgb[1] = _rgb[1] * 255.0f;
  _rgb[2] = _rgb[2] * 255.0f;

  rgb[0] = static_cast<unsigned char>(_rgb[0]);
  rgb[1] = static_cast<unsigned char>(_rgb[1]);
  rgb[2] = static_cast<unsigned char>(_rgb[2]);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void HexagonalOps::generateRodriguesColor(float r1, float r2, float r3, unsigned char* rgb)
{
  float range1 = 2.0f*HexDim1InitValue;
  float range2 = 2.0f*HexDim2InitValue;
  float range3 = 2.0f*HexDim3InitValue;
  float max1 = range1/2.0f;
  float max2 = range2/2.0f;
  float max3 = range3/2.0f;
  float red = (r1+max1)/range1;
  float green = (r2+max2)/range2;
  float blue = (r3+max3)/range3;

  // Scale values from 0 to 1.0
  red = red / max1;
  green = green / max1;
  blue = blue / max2;

  // Multiply by 255 to get an R/G/B value
  red = red * 255.0f;
  green = green * 255.0f;
  blue = blue * 255.0f;

  rgb[0] = static_cast<unsigned char> (red);
  rgb[1] = static_cast<unsigned char> (green);
  rgb[2] = static_cast<unsigned char> (blue);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<UInt8ArrayType::Pointer> HexagonalOps::generatePoleFigure(FloatArrayType* eulers, int poleFigureDimension, int lambertDimension, int numColors)
{
  std::vector<UInt8ArrayType::Pointer> poleFigures;

  int numOrientations = eulers->GetNumberOfTuples();

  // Create an Array to hold the XYZ Coordinates which are the coords on the sphere.
  // this is size for CUBIC ONLY, <001> Family
  FloatArrayType::Pointer xyz001 = FloatArrayType::CreateArray(numOrientations * 2, 3, "Hex_<001>_xyzCoords");
  // this is size for CUBIC ONLY, <011> Family
  FloatArrayType::Pointer xyz011 = FloatArrayType::CreateArray(numOrientations * 6, 3, "Hex_<011>_xyzCoords");
  // this is size for CUBIC ONLY, <111> Family
  FloatArrayType::Pointer xyz111 = FloatArrayType::CreateArray(numOrientations * 6, 3, "Hex_<111>_xyzCoords");


  float sphereRadius = 1.0f;

  // Generate the coords on the sphere
  generateSphereCoordsFromEulers(eulers, xyz001.get(), xyz011.get(), xyz111.get());

  // These arrays hold the "intensity" images which eventually get converted to an actual Color RGB image
  // Generate the modified Lambert projection images (Squares, 2 of them, 1 for northern hemisphere, 1 for southern hemisphere
  ModifiedLambertProjection::Pointer lambert = ModifiedLambertProjection::CreateProjectionFromXYZCoords(xyz001.get(), lambertDimension, sphereRadius);

  // Now create the intensity image that will become the actual Pole figure image
  {
    DoubleArrayType::Pointer intensity001 = lambert->createStereographicProjection(poleFigureDimension);
    intensity001->SetName("PoleFigure_<001>");
    UInt8ArrayType::Pointer image = ImageUtilities::CreateColorImage(intensity001.get(), poleFigureDimension, poleFigureDimension, numColors, "Hex PoleFigure <0001>");
    poleFigures.push_back(image);
  }

  // Generate the <011> pole figure which will generate a new set of Lambert Squares
  {
    lambert = ModifiedLambertProjection::CreateProjectionFromXYZCoords(xyz011.get(), lambertDimension, sphereRadius);
    DoubleArrayType::Pointer intensity011 = lambert->createStereographicProjection(poleFigureDimension);
    intensity011->SetName("PoleFigure_<011>");
    UInt8ArrayType::Pointer image = ImageUtilities::CreateColorImage(intensity011.get(), poleFigureDimension, poleFigureDimension, numColors, "Hex PoleFigure <1010>");
    poleFigures.push_back(image);
  }
  // Generate the <111> pole figure which will generate a new set of Lambert Squares
  {
    lambert = ModifiedLambertProjection::CreateProjectionFromXYZCoords(xyz111.get(), lambertDimension, sphereRadius);
    DoubleArrayType::Pointer intensity111 = lambert->createStereographicProjection(poleFigureDimension);
    intensity111->SetName("PoleFigure_<111>");
    UInt8ArrayType::Pointer image = ImageUtilities::CreateColorImage(intensity111.get(), poleFigureDimension, poleFigureDimension, numColors, "Hex PoleFigure <1120>");
    poleFigures.push_back(image);
  }

  return poleFigures;
}
