/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "ImportASCIIDataWizard.h"

#include <QtCore/QFile>

#include "DelimitedOrFixedWidthPage.h"
#include "DelimitedPage.h"
#include "DataFormatPage.h"
#include "ASCIIDataModel.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImportASCIIDataWizard::ImportASCIIDataWizard(const QString &inputFilePath, QWidget* parent) :
  QWizard(parent),
  m_InputFilePath(inputFilePath)
{
  setWindowTitle("ASCII Data Import Wizard");
  setOptions(QWizard::NoBackButtonOnStartPage | QWizard::HaveHelpButton);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  resize(721, 683);

  m_RefreshBtn = new QPushButton("Refresh", this);
  connect(m_RefreshBtn, SIGNAL(pressed()), this, SLOT(refreshModel()));
  setButton(QWizard::HelpButton, m_RefreshBtn);

  DelimitedOrFixedWidthPage* dOrFPage = new DelimitedOrFixedWidthPage(inputFilePath, this);
  setPage(DelimitedOrFixedWidth, dOrFPage);

  DelimitedPage* dPage = new DelimitedPage(inputFilePath, this);
  setPage(Delimited, dPage);

  DataFormatPage* dfPage = new DataFormatPage(inputFilePath, this);
  setPage(DataFormat, dfPage);

#ifndef Q_OS_MAC
  setWizardStyle(ModernStyle);
#else
  setWizardStyle(MacStyle);
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImportASCIIDataWizard::~ImportASCIIDataWizard()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImportASCIIDataWizard::ReadLine(const QString &inputFilePath, int line)
{
  QStringList lines = ReadLines(inputFilePath, line, 1);

  if (lines.size() != 1)
  {
    return QString();
  }
  
  return lines[0];
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList ImportASCIIDataWizard::ReadLines(const QString &inputFilePath, int beginLine, int numOfLines)
{
  QStringList result;

  QFile inputFile(inputFilePath);
  if (inputFile.open(QIODevice::ReadOnly))
  {
    QTextStream in(&inputFile);

    for (int i = 0; i < beginLine + numOfLines - 1; i++)
    {
      while (i < beginLine - 1)
      {
        // Skip all lines before "value"
        QString line = in.readLine();
        i++;
      }

      QString line = in.readLine();
      result.push_back(line);
    }
    inputFile.close();
  }

  return result;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<QStringList> ImportASCIIDataWizard::TokenizeLines(QStringList lines, bool isFixedWidth, bool tabAsDelimiter, bool semicolonAsDelimiter, bool commaAsDelimiter, bool spaceAsDelimiter, bool consecutiveDelimiters)
{
  QString expStr = "";
  if (isFixedWidth == true)
  {
    expStr.append(" |\\t|");
  }
  if (tabAsDelimiter == true)
  {
    expStr.append("\\t|");
  }
  if (semicolonAsDelimiter == true)
  {
    expStr.append(";|");
  }
  if (commaAsDelimiter == true)
  {
    expStr.append(",|");
  }
  if (spaceAsDelimiter == true)
  {
    expStr.append(" |");
  }

  expStr.chop(1);

  QRegularExpression exp(expStr);

  QList<QStringList> tokenizedLines;
  for (int row = 0; row < lines.size(); row++)
  {
    QString line = lines[row];

    QStringList tokenizedLine;
    if (expStr.isEmpty() == true)
    {
      tokenizedLine.push_back(line);
    }
    else if (consecutiveDelimiters == true || isFixedWidth == true)
    {
      tokenizedLine = line.split(exp, QString::SkipEmptyParts);
    }
    else
    {
      tokenizedLine = line.split(exp, QString::KeepEmptyParts);
    }

    tokenizedLines.push_back(tokenizedLine);
  }

  return tokenizedLines;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportASCIIDataWizard::InsertTokenizedLines(QList<QStringList> tokenizedLines, int firstRowHeaderIndex)
{
  ASCIIDataModel* model = ASCIIDataModel::Instance();
  model->clearContents();

  int vHeaderIndex = firstRowHeaderIndex;

  for (int row = 0; row < tokenizedLines.size(); row++)
  {
    QStringList tokenizedLine = tokenizedLines[row];

    while (model->columnCount() < tokenizedLine.size())
    {
      model->insertColumn(model->columnCount());
    }

    for (int column = 0; column < tokenizedLine.size(); column++)
    {
      QString token = tokenizedLine[column];
      QModelIndex index = model->index(row, column);
      model->setData(index, token, Qt::DisplayRole);
    }

    model->setHeaderData(row, Qt::Vertical, QString::number(vHeaderIndex), Qt::DisplayRole);
    vHeaderIndex++;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportASCIIDataWizard::InsertLines(QStringList lines, int firstRowHeaderIndex)
{
  ASCIIDataModel* model = ASCIIDataModel::Instance();

  model->insertColumn(0);
  for (int row = 0; row < lines.size(); row++)
  {
    QString line = lines[row];
    QModelIndex index = model->index(row, 0);
    model->setData(index, line, Qt::DisplayRole);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportASCIIDataWizard::LoadOriginalLines(QStringList lines)
{
  ASCIIDataModel* model = ASCIIDataModel::Instance();
  
  if (model->rowCount() > 0)
  {
    model->removeRows(0, model->rowCount());
  }

  for (int i = 0; i < lines.size(); i++)
  {
    QString line = lines[i];

    int row = model->rowCount();
    model->insertRow(row);
    model->setOriginalString(row, line);
    model->setHeaderData(row, Qt::Vertical, QString::number(i + 1), Qt::DisplayRole);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportASCIIDataWizard::refreshModel()
{
  AbstractWizardPage* page = dynamic_cast<AbstractWizardPage*>(currentPage());
  page->refreshModel();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportASCIIDataWizard::setInputFilePath(const QString &inputFilePath)
{
  m_InputFilePath = inputFilePath;
}
