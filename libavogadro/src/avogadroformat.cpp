/**********************************************************************
  AvogadroFormat - Open Babel XML module for saving Avogadro-specific data

  Copyright (C) 2008 Geoffrey R. Hutchison

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include <openbabel/babelconfig.h>
#include <openbabel/xml.h>

#include <QDebug>

using namespace std;

namespace OpenBabel
{

  class AvogadroFormat : public XMLMoleculeFormat
  {
  public:
    AvogadroFormat() 
    {
      OBConversion::RegisterFormat("aml", this);
      XMLConversion::RegisterXMLFormat(this);
    }
    virtual const char* NamespaceURI() const
    { return "http://avogadro.openmolecules.net/"; }
    virtual const char* SpecificationURL()
    { return "http://avogadro.openmolecules.net/wiki/AvogadroFormat"; }
    virtual const char* GetMIMEType() 
    { return "chemical/x-avogadro-xml"; }
    virtual unsigned int Flags()
    { return (READXML | NOTWRITABLE); }

    virtual const char* Description()
    {
      return " \
Avogadro format \n \
\n";
    };

    virtual bool DoElement(const string& name);
    virtual bool EndElement(const string& name);

    // EndTag is used so that the stream buffer is is filled with the XML from
    // complete objects, as far as possible. 
    virtual const char* EndTag(){ return "/Avogadro>"; };

  private:

  };

  ////////////////////////////////////////////////////////////////////

  AvogadroFormat theAvogadroFormat;

  ////////////////////////////////////////////////////////////////////

  bool AvogadroFormat::DoElement(const std::string& name)
  {
    qDebug() << " End Element " << name.c_str();

    return true;
  }

  bool AvogadroFormat::EndElement(const std::string& name)
  {
    qDebug() << " End Element " << name.c_str();

    return true;
  }	

}//namespace
