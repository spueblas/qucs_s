/***************************************************************************
                               sparamfile.cpp
                              ----------------
    begin                : Sat Aug 23 2003
    copyright            : (C) 2003 by Michael Margraf
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
#include "sparamfile.h"
#include "main.h"
#include "misc.h"
#include "node.h"

#include <QFileInfo>


SParamFile::SParamFile()
{
  Description = QObject::tr("S parameter file");
  Simulator = spicecompat::simAll;

  Model = "SPfile";
  Name  = "X";
  SpiceModel = "X";

  // must be the first property !!!
  Props.append(new Property("File", "test.s1p", true,
		QObject::tr("name of the s parameter file")));
  Props.append(new Property("Data", "rectangular", false,
		QObject::tr("data type")+" [rectangular, polar]"));
  Props.append(new Property("Interpolator", "linear", false,
		QObject::tr("interpolation type")+" [linear, cubic]"));
  Props.append(new Property("duringDC", "open", false,
		QObject::tr("representation during DC analysis")+
			    " [open, short, shortall, unspecified]"));

  // must be the last property !!!
  Props.append(new Property("Ports", "1", false,
		QObject::tr("number of ports")));

  createSymbol();
}

// -------------------------------------------------------
Component* SParamFile::newOne()
{
  SParamFile* p = new SParamFile();
  p->Props.back()->Value = Props.back()->Value;
  p->recreate(0);
  return p;
}

// -------------------------------------------------------
Element* SParamFile::info(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("n-port S parameter file");
  BitmapFile = (char *) "spfile3";

  if(getNewOne) {
    SParamFile* p = new SParamFile();
    p->Props.front()->Value = "test.s3p";
    p->Props.back()->Value = "3";
    p->recreate(0);
    return p;
  }
  return 0;
}

// -------------------------------------------------------
Element* SParamFile::info1(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("1-port S parameter file");
  BitmapFile = (char *) "spfile1";

  if(getNewOne)  return new SParamFile();
  return 0;
}

// -------------------------------------------------------
Element* SParamFile::info2(QString& Name, char* &BitmapFile, bool getNewOne)
{
  Name = QObject::tr("2-port S parameter file");
  BitmapFile = (char *) "spfile2";

  if(getNewOne) {
    SParamFile* p = new SParamFile();
    p->Props.front()->Value = "test.s2p";
    p->Props.back()->Value = "2";
    p->recreate(0);
    return p;
  }
  return 0;
}

// -------------------------------------------------------
QString SParamFile::getSubcircuitFile()
{
  return misc::properAbsFileName(Props.front()->Value, containingSchematic);

  // construct full filename
  QString FileName = Props.front()->Value;
  return misc::properAbsFileName(FileName);
}

// -------------------------------------------------------
QString SParamFile::netlist()
{
  QString s = Model+":"+Name;

  // output all node names
  for (Port *p1 : Ports)
    s += " "+p1->Connection->Name;   // node names

  // output all properties
  s += " "+Props.at(0)->Name+"=\"{"+getSubcircuitFile()+"}\"";

  // data type
  s += " "+Props.at(1)->Name+"=\""+Props.at(1)->Value+"\"";

  // interpolator type
  s += " "+Props.at(2)->Name+"=\""+Props.at(2)->Value+"\"";

  // DC property
  s += " "+Props.at(3)->Name+"=\""+Props.at(3)->Value+"\"\n";

  return s;
}

// -------------------------------------------------------
void SParamFile::createSymbol()
{
  QFont Font(QucsSettings.font); // default application font
  // symbol text is smaller (10 pt default)
  Font.setPointSize(10 ); 
  // get the small font size; use the screen-compatible metric
  QFontMetrics  smallmetrics(Font, 0);
  int fHeight = smallmetrics.lineSpacing();
  QString stmp;

  int w, PortDistance = 60;
  int Num = Props.back()->Value.toInt();

  // adjust number of ports
  if(Num < 1) Num = 1;
  else if(Num > 8) {
    PortDistance = 20;
    if(Num > 40) Num = 40;
  }
  Props.back()->Value = QString::number(Num);

  // draw symbol outline
  int h = (PortDistance/2)*((Num-1)/2) + 15;
  Lines.append(new qucs::Line(-15, -h, 15, -h,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line( 15, -h, 15,  h,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line(-15,  h, 15,  h,QPen(Qt::darkBlue,2)));
  Lines.append(new qucs::Line(-15, -h,-15,  h,QPen(Qt::darkBlue,2)));
  stmp = QObject::tr("file"); 
  w = smallmetrics.boundingRect(stmp).width(); // compute text size to center it
  Texts.append(new Text(-w/2, -fHeight/2, stmp));

  int i=0, y = 15-h;
  while(i<Num) { // add ports lines and numbers
    i++;
    Lines.append(new qucs::Line(-30, y,-15, y,QPen(Qt::darkBlue,2)));
    Ports.append(new Port(-30, y));
    stmp = QString::number(i);
    w = smallmetrics.boundingRect(stmp).width();
    Texts.append(new Text(-25-w, y-fHeight-2, stmp)); // text right-aligned

    if(i == Num) break; // if odd number of ports there will be one port less on the right side
    i++;
    Lines.append(new qucs::Line( 15, y, 30, y,QPen(Qt::darkBlue,2)));
    Ports.append(new Port( 30, y));
    stmp = QString::number(i);
    Texts.append(new Text(25, y-fHeight-2, stmp)); // text left-aligned
    y += PortDistance;
  }

  Lines.append(new qucs::Line( 0, h, 0,h+15,QPen(Qt::darkBlue,2)));
  Texts.append(new Text( 4, h,"Ref"));
  Ports.append(new Port( 0,h+15));    // 'Ref' port

  x1 = -30; y1 = -h-2;
  x2 =  30; y2 =  h+15;
  // compute component name text position - normal size font
  QFontMetrics  metrics(QucsSettings.font, 0);   // use the screen-compatible metric
  tx = x1+4;
  ty = y1 - 2*metrics.lineSpacing() - 4;
}

QString SParamFile::spice_netlist(spicecompat::SpiceDialect dialect /* = spicecompat::SPICEDefault */)
{
    QString s;
    if (dialect == spicecompat::SPICEXyce) {
        int Np = getProperty("Ports")->Value.toInt();
        s = "YLIN YLIN_" + Name;
        QString s_mod = "YLIN_" + Name + "_model";
        for(int i = 0; i < Np; i++) {
            QString p_in = spicecompat::normalize_node_name(Ports.at(i)->Connection->Name);
            QString p_com = spicecompat::normalize_node_name(Ports.at(Np)->Connection->Name);
            s += QStringLiteral(" %1 %2").arg(p_in).arg(p_com);
        }
        s += QStringLiteral(" %1\n").arg(s_mod);
        s += QStringLiteral(".MODEL %1 LIN TSTONEFILE=%2\n").arg(s_mod)
                .arg(getSubcircuitFile());
    } else {
        s += SpiceModel+Name;
        for (Port *p1 : Ports) {
            s += " "+ spicecompat::normalize_node_name(p1->Connection->Name);   // node names
        }
        s += " Sub_" + Model + "_" + Name + "\n";
    }
    return s;
}
