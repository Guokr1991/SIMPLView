/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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

#include "FindVolFractions.h"

#include <sstream>

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/DataArrays/IDataArray.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindVolFractions::FindVolFractions() :
  AbstractFilter(),
  m_CellEnsembleAttributeMatrixName(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellEnsembleAttributeMatrixName, ""),
  m_CellPhasesArrayPath(DREAM3D::Defaults::VolumeDataContainerName, DREAM3D::Defaults::CellAttributeMatrixName, DREAM3D::CellData::Phases),
  m_VolFractionsArrayName(DREAM3D::EnsembleData::VolFractions),
  m_CellPhasesArrayName(DREAM3D::CellData::Phases),
  m_CellPhases(NULL),
  m_VolFractions(NULL)
{

  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindVolFractions::~FindVolFractions()
{
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindVolFractions::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FilterParameter::New("Required Information", "", FilterParameterWidgetType::SeparatorWidget, "", true));
  parameters.push_back(FilterParameter::New("CellPhases", "CellPhasesArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getCellPhasesArrayPath(), true, ""));
  parameters.push_back(FilterParameter::New("Cell Ensemble Attribute Matrix Name", "CellEnsembleAttributeMatrixName", FilterParameterWidgetType::AttributeMatrixSelectionWidget, getCellEnsembleAttributeMatrixName(), true, ""));
  parameters.push_back(FilterParameter::New("Created Information", "", FilterParameterWidgetType::SeparatorWidget, "", true));
  parameters.push_back(FilterParameter::New("VolFractions", "VolFractionsArrayName", FilterParameterWidgetType::StringWidget, getVolFractionsArrayName(), true, ""));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
void FindVolFractions::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setCellEnsembleAttributeMatrixName(reader->readDataArrayPath("CellEnsembleAttributeMatrixName", getCellEnsembleAttributeMatrixName()));
  setVolFractionsArrayName(reader->readString("VolFractionsArrayName", getVolFractionsArrayName() ) );
  setCellPhasesArrayPath(reader->readDataArrayPath("CellPhasesArrayPath", getCellPhasesArrayPath() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int FindVolFractions::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("CellEnsembleAttributeMatrixName", getCellEnsembleAttributeMatrixName());
  writer->writeValue("VolFractionsArrayName", getVolFractionsArrayName() );
  writer->writeValue("CellPhasesArrayPath", getCellPhasesArrayPath() );
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindVolFractions::dataCheck()
{
  DataArrayPath tempPath;
  setErrorCondition(0);

  QVector<size_t> dims(1, 1);
  m_CellPhasesPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, getCellPhasesArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_CellPhasesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_CellPhases = m_CellPhasesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  tempPath.update(getCellEnsembleAttributeMatrixName().getDataContainerName(), getCellEnsembleAttributeMatrixName().getAttributeMatrixName(), getVolFractionsArrayName() );
  m_VolFractionsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_VolFractionsPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_VolFractions = m_VolFractionsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindVolFractions::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindVolFractions::execute()
{
  setErrorCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  int64_t totalPoints = m_CellPhasesPtr.lock()->getNumberOfTuples();
  size_t totalEnsembles = m_VolFractionsPtr.lock()->getNumberOfTuples();

  for(size_t i = 1; i < totalEnsembles; i++)
  {
    m_VolFractions[i] = 0;
  }
  for(size_t i = 0; i < totalPoints; i++)
  {
    m_VolFractions[m_CellPhases[i]]++;
  }

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindVolFractions::newFilterInstance(bool copyFilterParameters)
{
  FindVolFractions::Pointer filter = FindVolFractions::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}
