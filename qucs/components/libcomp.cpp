/***************************************************************************
                               libcomp.cpp
                              -------------
    begin                : Fri Jun 10 2005
    copyright            : (C) 2005 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "libcomp.h"
#include "main.h"
#include "misc.h"
#include "node.h"
#include "extsimkernels/qucs2spice.h"
#include "extsimkernels/spicecompat.h"


#include <QTextStream>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>

LibComp::LibComp()
{
  Type = isComponent;   // both analog and digital
  Description = QObject::tr("Component taken from Qucs library");

  Ports.append(new Port(0,  0));  // dummy port because of being device

  Model = "Lib";
  Name  = "X";
  SpiceModel = "X";

  Props.append(new Property("Lib", "", true,
		QObject::tr("name of qucs library file")));
  Props.append(new Property("Comp", "", true,
		QObject::tr("name of component in library")));
}

// ---------------------------------------------------------------------
Component* LibComp::newOne()
{
  LibComp *p = new LibComp();
  p->Props.at(0)->Value = Props.at(0)->Value;
  p->Props.at(1)->Value = Props.at(1)->Value;
  p->recreate(0);
  return p;
}

// ---------------------------------------------------------------------
// Makes the schematic symbol subcircuit with the correct number
// of ports.
void LibComp::createSymbol()
{
  tx = INT_MIN;
  ty = INT_MIN;
  if(loadSymbol() > 0) {
    if(tx == INT_MIN)  tx = x1+4;
    if(ty == INT_MIN)  ty = y2+4;
  }
  else {
    // only paint a rectangle
    Lines.append(new qucs::Line(-15, -15, 15, -15, QPen(Qt::darkBlue,2)));
    Lines.append(new qucs::Line( 15, -15, 15,  15, QPen(Qt::darkBlue,2)));
    Lines.append(new qucs::Line(-15,  15, 15,  15, QPen(Qt::darkBlue,2)));
    Lines.append(new qucs::Line(-15, -15,-15,  15, QPen(Qt::darkBlue,2)));

    x1 = -18; y1 = -18;
    x2 =  18; y2 =  18;

    tx = x1+4;
    ty = y2+4;
  }
}

// ---------------------------------------------------------------------
// Loads the section with name "Name" from library file into "Section".
int LibComp::loadSection(const QString& Name, QString& Section,
             QStringList *Includes, QStringList *Attach)
{
  QDir Directory(QucsSettings.LibDir);
  QFile file(misc::properAbsFileName(Directory.absoluteFilePath(Props.at(0)->Value + ".lib"), containingSchematic));
  if(!file.open(QIODevice::ReadOnly))
    return -1;

  QString libDefaultSymbol;

  QTextStream ReadWhole(&file);
  Section = ReadWhole.readAll();
  file.close();


  if(Section.left(14) != "<Qucs Library ")  // wrong file type ?
    return -2;

  int Start, End = Section.indexOf(' ', 14);
  if(End < 15) return -3;
  QString Line = Section.mid(14, End-14); // extract version string
  VersionTriplet LibVersion = VersionTriplet(Line);
  if (LibVersion > QucsVersion) {// wrong version number ?
      if (!QucsSettings.IgnoreFutureVersion) {
          return -3;
      }
  }

  if(Name == "Symbol") {
    Start = Section.indexOf("\n<", 14); // library has default symbol
    if(Start > 0)
      if(Section.mid(Start+2, 14) == "DefaultSymbol>") {
        Start += 16;
        End = Section.indexOf("\n</DefaultSymbol>", Start);
        if(End < 0)  return -9;
        libDefaultSymbol = Section.mid(Start, End-Start);
      }
  }

  // search component
  Line = "\n<Component " + Props.at(1)->Value + ">";
  Start = Section.indexOf(Line);
  if(Start < 0)  return -4;  // component not found
  Start = Section.indexOf('\n', Start);
  if(Start < 0)  return -5;  // file corrupt
  Start++;
  End = Section.indexOf("\n</Component>", Start);
  if(End < 0)  return -6;  // file corrupt
  Section = Section.mid(Start, End-Start+1);
  
  // search model includes
  if(Includes) {
    int StartI, EndI;
    StartI = Section.indexOf("<"+Name+"Includes");
    if(StartI >= 0) {  // includes found
      StartI = Section.indexOf('"', StartI);
      if(StartI < 0)  return -10;  // file corrupt
      EndI = Section.indexOf('>', StartI);
      if(EndI < 0)  return -11;  // file corrupt
      StartI++; EndI--;
      QString inc = Section.mid(StartI, EndI-StartI);
      QStringList f = inc.split(QRegularExpression("\"\\s+\""));
      for(QStringList::Iterator it = f.begin(); it != f.end(); ++it ) {
	Includes->append(*it);
      }
    }
  }

  // search attached files
  if(Attach) {
    int StartI, EndI;
    StartI = Section.indexOf("<"+Name+"Attach");
    if(StartI >= 0) {  // includes found
      StartI = Section.indexOf('"', StartI);
      if(StartI < 0)  return -10;  // file corrupt
      EndI = Section.indexOf('>', StartI);
      if(EndI < 0)  return -11;  // file corrupt
      StartI++; EndI--;
      QString inc = Section.mid(StartI, EndI-StartI);
      QStringList f = inc.split(QRegularExpression("\"\\s+\""));
      for(QStringList::Iterator it = f.begin(); it != f.end(); ++it ) {
    Attach->append(*it);
      }
    }
  }

  // search model
  Start = Section.indexOf("<"+Name+">");
  if(Start < 0) {
    if((Name == "Symbol") && (!libDefaultSymbol.isEmpty())) {
      // component does not define its own symbol but the library defines a default symbol
      Section = libDefaultSymbol;
      return 0;
    } else {
      return -7;  // symbol not found
    }
  }
  Start = Section.indexOf('\n', Start);
  if(Start < 0)  return -8;  // file corrupt
  while(Section.at(++Start) == ' ') ;
  End = Section.indexOf("</"+Name+">", Start);
  if(End < 0)  return -9;  // file corrupt

  // snip actual model
  Section = Section.mid(Start, End-Start);
  return 0;
}

// ---------------------------------------------------------------------
// Loads the symbol for the subcircuit from the schematic file and
// returns the number of painting elements.
int LibComp::loadSymbol()
{
  int z, Result;
  QString FileString, Line;
  z = loadSection("Symbol", FileString);
  if(z < 0) {
    if(z != -7)  return z;

    // If library component not defined as subcircuit, then load
    // new component and transfer data to this component.
    z = loadSection("Model", Line);
    if(z < 0)  return z;

    std::shared_ptr<Component> pc(getComponentFromName(Line));
    if(!pc)  return -20;
    copyComponent(pc.get());

    return 1;
  }


  z  = 0;
  x1 = y1 = INT_MAX;
  x2 = y2 = INT_MIN;

  QTextStream stream(&FileString, QIODevice::ReadOnly);
  while(!stream.atEnd()) {
    Line = stream.readLine();
    Line = Line.trimmed();
    if(Line.isEmpty())  continue;
    if(Line.at(0) != '<') return -11;
    if(Line.at(Line.length()-1) != '>') return -12;
    Line = Line.mid(1, Line.length()-2); // cut off start and end character
    Result = analyseLine(Line, 2);
    if(Result < 0) return -13;   // line format error
    z += Result;
  }

  x1 -= 4;  x2 += 4;   // enlarge component boundings a little
  y1 -= 4;  y2 += 4;
  return z;      // return number of ports
}

// -------------------------------------------------------
QString LibComp::getSubcircuitFile()
{
  QDir Directory(QucsSettings.LibDir);
  QString FileName = misc::properAbsFileName(Directory.absoluteFilePath(Props.first()->Value) + ".lib");
  FileName.chop(4);
  return FileName;
}

// -------------------------------------------------------
bool LibComp::createSubNetlist(QTextStream *stream, QStringList &FileList,
			       int type)
{
  int r = -1;
  QString FileString;
  QStringList Includes;
  if(type&1) {
    r = loadSection("Model", FileString, &Includes);
  } else if(type&2) {
    r = loadSection("VHDLModel", FileString, &Includes);
  } else if(type&4) {
    r = loadSection("VerilogModel", FileString, &Includes);
  } else if(type&8) {
    r = loadSection("Spice",FileString, &Includes);
    if (r<0) {
        r = loadSection("Model", FileString, &Includes); // Ngspice
        FileString = qucs2spice::convert_netlist(FileString);
    }
  } else if (type&16) {
      r = loadSection("Spice",FileString, &Includes);
      if (r<0) {
          r = loadSection("Model", FileString, &Includes); // Ngspice
          FileString = qucs2spice::convert_netlist(FileString,true);
      }
  }
  if(r < 0)  return false;

  // also include files
  int error = 0;
  for(QStringList::Iterator it = Includes.begin();
      it != Includes.end(); ++it ) {
    QString s = getSubcircuitFile()+"/"+*it;
    if(FileList.indexOf(s) >= 0) continue;
    FileList.append(s);

    // load file and stuff into stream
    QFile file(s);
    if(!file.open(QIODevice::ReadOnly)) {
      error++;
    } else {
      QByteArray FileContent = file.readAll();
      file.close();
      //?stream->writeRawBytes(FileContent.data(), FileContent.size());
      (*stream) << FileContent.data();
      qDebug() << "hi from libcomp";
    }
  }

  (*stream) << "\n" << FileString << "\n";
  return error > 0 ? false : true;
}

// -------------------------------------------------------
QString LibComp::createType()
{
  QString Type = misc::properFileName(Props.at(0)->Value);
  return misc::properName(Type + "_" + Props.at(1)->Value);
}

// -------------------------------------------------------
QString LibComp::netlist()
{
  QString s = "Sub:"+Name;   // output as subcircuit

  // output all node names
  for (Port *p1 : Ports)
    s += " "+p1->Connection->Name;   // node names

  // output property
  s += " Type=\""+createType()+"\"";   // type for subcircuit

  // output user defined parameters
  for(int i = 2;i<Props.size();i++)
    s += " "+Props.at(i)->Name+"=\""+Props.at(i)->Value+"\"";

  return s + '\n';
}

// -------------------------------------------------------
QString LibComp::verilogCode(int)
{
  QString s = "  Sub_" + createType() + " " + Name + " (";

  // output all node names
  QListIterator<Port *> iport(Ports);
  Port *pp = iport.next();
  if(pp)  s += pp->Connection->Name;
  while (iport.hasNext()) {
    pp = iport.next();
    s += ", "+pp->Connection->Name;   // node names
  }

  s += ");\n";
  return s;
}

// -------------------------------------------------------
QString LibComp::vhdlCode(int)
{
  QString s = "  " + Name + ": entity Sub_" + createType() + " port map (";

  // output all node names
  QListIterator<Port *> iport(Ports);
  Port *pp = iport.next();
  if(pp)  s += pp->Connection->Name;
  while (iport.hasNext()) {
    pp = iport.next();
    s += ", "+pp->Connection->Name;   // node names
  }

  s += ");\n";
  return s;
}

QString LibComp::spice_netlist(spicecompat::SpiceDialect dialect /* = spicecompat::SPICEDefault */)
{
    Q_UNUSED(dialect);

    QString s = SpiceModel + Name + " 0 "; // connect ground of subckt to circuit ground
    for (Port *p1 : Ports)
      s += " "+p1->Connection->Name;   // node names
    s += " " + createType();

    // output user defined parameters
    for(int i = 2;i<Props.size();i++) {
      QString val = spicecompat::normalize_value(Props.at(i)->Value);
      s += " "+Props.at(i)->Name+"="+val;
    }
    s +="\n";

    return s;
}

QString LibComp::cdl_netlist()
{
    return spice_netlist(spicecompat::CDL);
}

QString LibComp::getSpiceLibrary()
{
  QStringList files;
  QString content;
  QStringList includes,attach;

  int r = loadSection("Spice",content,&includes,&attach);
  if (r<0) {
    return QString();
  }
  for (const auto &file : attach) {
    if (file.endsWith(".cir", Qt::CaseInsensitive) ||
        file.endsWith(".ckt", Qt::CaseInsensitive) ||
        file.endsWith(".lib", Qt::CaseInsensitive) ||
        file.endsWith(".sp", Qt::CaseInsensitive)) {
      files.append(getSubcircuitFile()+'/'+file);
    }
  }

  QString s;
  for (const auto &file: files) { // for netlist
    s += QStringLiteral(".INCLUDE \"%1\"\n").arg(file);
  }
  return s;
}
